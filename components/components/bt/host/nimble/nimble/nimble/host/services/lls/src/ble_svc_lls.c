/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <string.h>
#include "sysinit/sysinit.h"
#include "host/ble_hs.h"
#include "services/lls/ble_svc_lls.h"

#if MYNEWT_VAL(BLE_GATTS) && CONFIG_BT_NIMBLE_LLS_SERVICE
/* Callback function */
static ble_svc_lls_event_fn *ble_svc_lls_cb_fn;

/* Alert level */
static uint8_t ble_svc_lls_alert_level;

/* Write characteristic function */
static int
ble_svc_lls_chr_write(struct os_mbuf *om, uint16_t min_len,
                      uint16_t max_len, void *dst,
                      uint16_t *len);

/* Access function */
static int
ble_svc_lls_access(uint16_t conn_handle, uint16_t attr_handle,
                   struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def ble_svc_lls_defs[] = {
    {
        /*** Service: Link Loss Service (LLS). */
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = BLE_UUID16_DECLARE(BLE_SVC_LLS_UUID16),
        .characteristics = (struct ble_gatt_chr_def[]) { {
            /*** Characteristic: Alert Level. */
            .uuid = BLE_UUID16_DECLARE(BLE_SVC_LLS_CHR_UUID16_ALERT_LEVEL),
            .access_cb = ble_svc_lls_access,
            .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE,
        }, {
            0, /* No more characteristics in this service. */
        } },
    },

    {
        0, /* No more services. */
    },
};

/**
 * Writes the received value from a characteristic write to
 * the given destination.
 */
static int
ble_svc_lls_chr_write(struct os_mbuf *om, uint16_t min_len,
                      uint16_t max_len, void *dst,
                      uint16_t *len)
{
    uint16_t om_len;
    int rc;

    om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len) {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0) {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

/**
 * Simple read/write access callback for the alert level
 * characteristic.
 */
static int
ble_svc_lls_access(uint16_t conn_handle, uint16_t attr_handle,
                   struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    assert(ctxt->chr == &ble_svc_lls_defs[0].characteristics[0]);
    int rc;
    switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        rc = os_mbuf_append(ctxt->om, &ble_svc_lls_alert_level,
                            sizeof ble_svc_lls_alert_level);
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        rc = ble_svc_lls_chr_write(ctxt->om,
                                   sizeof ble_svc_lls_alert_level,
                                   sizeof ble_svc_lls_alert_level,
                                   &ble_svc_lls_alert_level, NULL);
        return rc;

    default:
        assert(0);
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

/**
 * This function is the crux of the link loss service. The application
 * developer must call this function inside the gap event callback
 * function when a BLE_GAP_EVENT_DISCONNECT event is received and
 * pass the disconnect reason into this function.
 *
 * Here, we then check if the disconnect reason is due to a timout, and if
 * so, we call the ble_svc_lls_event_fn callback with the current
 * alert level. The actual alert implementation is left up to the
 * developer.
 *
 * @param reason            The reason attatched to the GAP disconnect
 *                              event.
 */
void
ble_svc_lls_on_gap_disconnect(int reason)
{
    if (reason == BLE_HS_HCI_ERR(BLE_ERR_CONN_SPVN_TMO)) {
        if (ble_svc_lls_cb_fn != NULL) {
            ble_svc_lls_cb_fn(ble_svc_lls_alert_level);
        }
    }
}

/**
 * Gets the current alert level.
 *
 * @return The current alert level
 */
uint8_t
ble_svc_lls_alert_level_get(void)
{
    return ble_svc_lls_alert_level;
}

/**
 * Sets the current alert level.
 *
 * @return 0 on success, BLE_HS_EINVAL if the given alert level is not valid.
 */
int
ble_svc_lls_alert_level_set(uint8_t alert_level)
{
    if (alert_level > BLE_SVC_LLS_ALERT_LEVEL_HIGH_ALERT) {
        return BLE_HS_EINVAL;
    }

    memcpy(&ble_svc_lls_alert_level, &alert_level,
           sizeof alert_level);

    return 0;
}

void
ble_svc_lls_set_cb(ble_svc_lls_event_fn *cb)
{
    ble_svc_lls_cb_fn = cb;
}

/**
 * Initialize the IAS package.
 */
void
ble_svc_lls_init(void)
{
    int rc;

    /* Ensure this function only gets called by sysinit. */
    SYSINIT_ASSERT_ACTIVE();

    rc = ble_gatts_count_cfg(ble_svc_lls_defs);
    SYSINIT_PANIC_ASSERT(rc == 0);

    rc = ble_gatts_add_svcs(ble_svc_lls_defs);
    SYSINIT_PANIC_ASSERT(rc == 0);
}
#endif
