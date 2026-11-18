/**
 * @file rbf_clone.h
 * @brief Hub data import/export API (host validation; hardware validation pending).
 *
 * Normal-mode callbacks execute on the clone worker (default stack: 64 KiB).
 * Build the SDK with RBF_CLONE_THREAD_STACK_SIZE to override this budget.
 * Keep callbacks bounded; use heap storage or an application queue for large work.
 * The SDK schedules transfers internally. No application polling is required.
 * Data is opaque: the application owns storage and file validation. This API
 * does not promise selective import, merging, or simultaneous cloned hubs.
 */
#ifndef RBF_CLONE_H
#define RBF_CLONE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RBF_CLONE_DIRECTION_NONE = 0,
    RBF_CLONE_DIRECTION_EXPORT,
    RBF_CLONE_DIRECTION_IMPORT,
    RBF_CLONE_DIRECTION_FACTORY_RESET
} RBF_clone_direction_t;

typedef enum {
    RBF_CLONE_STATE_IDLE = 0,
    RBF_CLONE_STATE_ENTERING,
    RBF_CLONE_STATE_TRANSFERRING,
    RBF_CLONE_STATE_DATA_READY,
    RBF_CLONE_STATE_FINISHING,
    RBF_CLONE_STATE_CANCELLING,
    RBF_CLONE_STATE_RECOVERY_REQUIRED
} RBF_clone_state_t;

typedef enum {
    RBF_CLONE_EVT_MODE_READY = 0,
    RBF_CLONE_EVT_PROGRESS,
    RBF_CLONE_EVT_DATA_READY,
    RBF_CLONE_EVT_EXITED,
    RBF_CLONE_EVT_ERROR
} RBF_clone_evt_t;

typedef enum {
    RBF_CLONE_ERR_NONE = 0,
    RBF_CLONE_ERR_INVALID_ARGUMENT,
    RBF_CLONE_ERR_BUSY,
    RBF_CLONE_ERR_STORAGE,
    RBF_CLONE_ERR_PROTOCOL,
    RBF_CLONE_ERR_HUB,
    RBF_CLONE_ERR_TIMEOUT,
    RBF_CLONE_ERR_RESULT_UNKNOWN,
    RBF_CLONE_ERR_RESOURCE
} RBF_clone_error_t;

typedef enum {
    RBF_CLONE_EXIT_NONE = 0,
    RBF_CLONE_EXIT_COMPLETED,
    RBF_CLONE_EXIT_CANCELLED,
    RBF_CLONE_EXIT_ABNORMAL
} RBF_clone_exit_reason_t;

/** Scalar snapshot; safe to copy into an application event queue. */
typedef struct {
    RBF_clone_direction_t direction;
    RBF_clone_state_t state;
    uint16_t total_chunks;       /**< Export: zero until confirmed exit. */
    uint16_t confirmed_chunks;   /**< Unique chunks accepted, excluding retries. */
    RBF_clone_error_t error;
    uint8_t hub_error;           /**< Raw HUB error, valid for ERR_HUB only. */
    bool remote_exit_confirmed; /**< A local timeout does not set this flag. */
    RBF_clone_exit_reason_t exit_reason;
} RBF_clone_status_t;

typedef struct {
    RBF_clone_evt_t type;
    RBF_clone_status_t status;
} RBF_clone_event_t;

/** Explicit caller configuration; timing uses the platform time abstraction. */
typedef struct {
    uint32_t response_timeout_ms; /**< Command/data inactivity timeout: 1..INT32_MAX ms. */
    uint32_t mode_timeout_ms;     /**< Entry/exit wait timeout: 1..INT32_MAX ms. */
    uint16_t retry_limit;         /**< Maximum data retries; zero disables retries.
                                      Never automatically retry destructive entry. */
} RBF_clone_options_t;

/**
 * Callbacks run serially in an SDK-managed execution context, without holding
 * SDK state locks. Do not wait for the application thread or call start, finish,
 * cancel, or registration from a callback. Notify the application using a
 * thread-safe queue. Storage callbacks must complete promptly.
 */
typedef struct {
    void *user; /**< Application context; keep alive until confirmed exit. */

    /**
     * Export: copy/take ownership of one chunk before returning 0; otherwise
     * return nonzero. seq starts at 1 (SDK-local). total is 0 while exporting because the
     * HUB does not announce a total. Save seq, size and original bytes.
     * data is borrowed and valid only for this callback. Failed writes do not
     * count as confirmed chunks. Required for export.
     */
    int (*write_chunk)(void *user, uint16_t total, uint16_t seq,
                       const uint8_t *data, uint16_t size);

    /**
     * Import: fill buffer without exceeding capacity and set size, then return
     * 0; return nonzero on missing data or failure. seq starts at 1. Repeated
     * requests for a seq must return identical data. Required for import.
     * Non-final chunks must be 234 bytes; the final chunk is 1..234 bytes.
     * buffer is SDK-owned and valid only during the callback.
     */
    int (*read_chunk)(void *user, uint16_t seq, uint8_t *buffer,
                      uint16_t capacity, uint16_t *size);

    /** Borrowed scalar event snapshot; copy it before returning. Required. */
    void (*event)(void *user, const RBF_clone_event_t *event);
} RBF_clone_evt_callbacks_t;

/**
 * Copy the callback cluster. Reject NULL, missing event callback, and changes
 * while a session is active or recovery is pending. The user context remains
 * application-owned. Returns 0 on success, -1 on failure.
 */
int rbf_clone_register_evt_callback(const RBF_clone_evt_callbacks_t *cbs);

/**
 * Request export mode; the HUB reboots. Requires initialized SDK, write/event
 * callbacks and non-NULL valid options. SDK copies options before returning.
 * Return 0 means accepted, not complete; -1 means rejected without starting.
 * Concurrent clone or OTA sessions are rejected. With current firmware export
 * completes automatically and preserves registered devices. Older firmware may
 * clear the source; verify firmware compatibility before use. No DATA_READY/finish gate exists for export; await EXITED.
 */
int rbf_clone_export_start(const RBF_clone_options_t *options);

/**
 * Request import mode. WARNING: import replaces target HUB data.
 * Validate the source in the application BEFORE calling. total must be nonzero;
 * storage must support rereading each original chunk until confirmed exit.
 * Requires read/event callbacks; other start contracts match export_start.
 */
int rbf_clone_import_start(uint16_t total, const RBF_clone_options_t *options);

/**
 * Import only: explicitly finish after DATA_READY. Export calls are rejected.
 * The HUB validates completeness and exits normally or abnormally; there is no
 * missing-chunk repair protocol. Wait for EXITED and inspect its reason.
 * Returns 0 if accepted, -1 for invalid state or request failure.
 */
int rbf_clone_finish(void);

/**
 * Explicit destructive factory reset (0x85). reboot is 0 or 1; invalid values
 * are rejected. Requires event callback and options; shares clone/OTA exclusion.
 * Return 0 means queued, not complete. No automatic retry. EXITED/COMPLETED
 * means a successful HUB ACK, not proof of reboot or erased device inventory;
 * verify these separately after the operation. Errors/timeouts use clone status.
 * Cancellation is not supported once queued. Call from application context.
 */
int rbf_hub_factory_reset_start(uint8_t reboot, const RBF_clone_options_t *options);

/**
 * Explicit recovery probe after factory-reset error/timeout only. Sends 60/00
 * (stop local enrollment); this also ends any active local enrollment session.
 * Only a successful ACK confirms normal mode and emits EXITED/ABNORMAL while
 * retaining the reset error (reset success remains unproven). A late successful
 * 85 ACK instead emits EXITED/COMPLETED. Errors or missing ACK keep exclusion;
 * call again to retry the non-destructive probe. Never resends factory reset.
 * Returns 0 when queued, -1 for wrong state, pending probe or callback reentry.
 */
int rbf_hub_factory_reset_recover(void);

/**
 * Request cancellation and wait asynchronously for confirmed exit. Export
 * cancellation does not factory-reset the source under the protocol. Import
 * cancellation cannot promise restoration of the prior device database.
 * Returns 0 if accepted, -1 if rejected. Timeout is not confirmed recovery.
 */
int rbf_clone_cancel(void);

/**
 * Copy a synchronized status snapshot; returns 0, or -1 for NULL status.
 * A session with an unknown remote result remains RECOVERY_REQUIRED and blocks
 * new starts until SDK observes exit or confirms normal mode. The application
 * must keep callback storage/context valid through that recovery period.
 */
int rbf_clone_status_get(RBF_clone_status_t *status);

#ifdef __cplusplus
}
#endif
#endif
