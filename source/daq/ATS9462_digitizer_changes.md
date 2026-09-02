# IOMMU DMA PTE Leak Workaround — Code Changes

**Date**: 2026-07-07
**File**: `fast_daq/source/daq/ATS9462_digitizer.cc`
**Related**: [debugging_report.md](debugging_report.md)

## Problem

The `ats9462` vendor kernel driver has a bug: `AlazarAbortAsyncRead` stops the DMA engine but does not call `dma_unmap_sg_attrs` on the outstanding DMA buffers. IOMMU page table entries (PTEs) are left in place. On the next run, `AlazarPostAsyncBuffer` → `dma_buffer_map_sg` → `intel_iommu_map_pages` finds the stale PTEs still set and emits `DMAR: ERROR: DMA PTE for vPFN … already set`. After ~240 runs the accumulated IOMMU page table corruption crashes the node.

The digitizer already calls `AlazarAbortAsyncRead` at the end of every run (self-termination when `f_buffers_completed >= buffers_per_acquisition()`). There is no missing stop command — the driver simply ignores the unmap step.

## Changes

### 1. Buffer pool recycled between runs (line 155–172)

**What**: After `AlazarAbortAsyncRead`, call `clear_buffers()` + `allocate_buffers()` to free the old DMA buffer pool and allocate a fresh one before marking the run complete.

**Why it works**: `clear_buffers()` calls `free()` on each buffer. `allocate_buffers()` calls `valloc()` to create new buffers. `valloc()` returns different virtual addresses than the just-freed ones (glibc does not immediately reuse freed addresses). When the next run's `commence_buffer_collection()` → `AlazarPostAsyncBuffer` → `dma_buffer_map_sg` fires, it maps the new virtual addresses, creating fresh IOMMU PTEs at new IOVAs. No collision with the stale PTEs → no DMAR errors.

**Tradeoff**: The stale PTEs for the old virtual addresses still leak in the IOMMU page tables. Combined with the reduced `dma-buffer-count`, this grows at ~4 KB/run — sustainable for long tests.

```diff
                     if ( f_buffers_completed >= buffers_per_acquisition() )
                     {
                         LINFO( flog, "All requested buffers ("<<f_buffers_completed<<") completed, ..." );
                         std::shared_ptr< daq_control > t_daq_control = ...;
                         t_daq_control->stop_run();
                         check_return_code_macro( AlazarAbortAsyncRead, f_board_handle );
+                        // Workaround for ats9462 driver IOMMU DMA PTE leak:
+                        // AlazarAbortAsyncRead does not call dma_unmap_sg_attrs, so
+                        // IOMMU PTEs accumulate across runs.  Freeing and reallocating
+                        // the buffer pool gives AlazarPostAsyncBuffer fresh virtual
+                        // addresses, avoiding PTE collisions on the next run.
+                        LINFO( flog, "recycling DMA buffer pool to avoid IOMMU PTE collisions" );
+                        clear_buffers();
+                        allocate_buffers();
+                        LINFO( flog, "DMA buffer pool recycled (" << f_board_buffers.size() << " buffers)" );
                         f_stopping = true;
                     }
```

### 2. Default `dma-buffer-count` reduced: 4883 → 5 (line 54)

**What**: Changed the constructor default for `f_dma_buffer_count`.

**Why**: Each posted DMA buffer creates ~100 IOMMU PTEs (one per 4 KB page of the buffer). At the original 4883 buffers, each run leaked ~488,000 stale PTEs. At 5 buffers, each run leaks ~500 stale PTEs — a 1000× reduction. Combined with the buffer recycling, the per-run IOMMU page table leak drops from ~4 MB to ~4 KB.

**Tradeoff**: 5 buffers × 409 KB each at 50 MSPS gives ~20 ms of DMA headroom. If the process is descheduled longer than that, the overrun recovery path (`process_a_buffer()`, line 354) triggers — it flushes buffers and restarts digitization, which loses a fraction of a second of data but is non-fatal.

```diff
-        f_dma_buffer_count( 4883 ),
+        f_dma_buffer_count( 5 ),  // was 4883; reduced to mitigate IOMMU DMA PTE leak per run
```

## Expected behavior after changes

| Metric | Before | After |
|--------|--------|-------|
| DMAR errors per run | ~3.5 | 0 (no PTE collisions) |
| IOMMU PTE leak per run | ~488,000 PTEs (~4 MB) | ~500 PTEs (~4 KB) |
| Runs before IOMMU exhaustion | ~240 | >100,000 (estimated) |
| DMA buffer memory | ~2 GB | ~2 MB |
| Overrun risk | Very low (2 GB headroom) | Moderate (20 ms headroom) |

## Revert plan

If the overrun rate is unacceptable, increase `dma-buffer-count` incrementally (8, 10, 16) until overruns stop. Each doubling increases the leak rate proportionally, but even at 50 buffers the leak is still 100× smaller than the original 4883.

When the vendor provides a fixed `ats9462.ko` that properly calls `dma_unmap_sg_attrs` in the abort path, revert both changes: remove the `clear_buffers()`/`allocate_buffers()` block and restore `f_dma_buffer_count( 4883 )`.
