/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file has the DFS dispatcher API which is exposed to outside of DFS
 * component.
 */

#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
/**
 * ucfg_dfs_set_postnol_freq() - Set PostNOL freq.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: User configured freq to switch to, post NOL, in MHZ.
 *
 */
QDF_STATUS ucfg_dfs_set_postnol_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t postnol_freq);

/**
 * ucfg_dfs_set_postnol_mode() - Set PostNOL mode.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_mode: User configured mode to switch to, post NOL, in MHZ.
 *
 */
QDF_STATUS ucfg_dfs_set_postnol_mode(struct wlan_objmgr_pdev *pdev,
				     uint16_t postnol_mode);

/**
 * ucfg_dfs_set_postnol_cfreq2() - Set PostNOL secondary center frequency.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: User configured secondary center frequency to switch to,
 * post NOL, in MHZ.
 *
 */
QDF_STATUS ucfg_dfs_set_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
				       qdf_freq_t postnol_cfreq2);

/**
 * ucfg_dfs_get_postnol_freq() - Get PostNOL freq.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: Pointer to user configured freq to switch to, post NOL.
 *
 */
QDF_STATUS ucfg_dfs_get_postnol_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t *postnol_freq);

/**
 * ucfg_dfs_get_postnol_mode() - Set PostNOL mode.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_mode: Pointer to user configured mode to switch to, post NOL.
 *
 */
QDF_STATUS ucfg_dfs_get_postnol_mode(struct wlan_objmgr_pdev *pdev,
				     uint8_t *postnol_mode);

/**
 * ucfg_dfs_get_postnol_cfreq2() - Set PostNOL secondary center frequency.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: Pointer to user configured secondary center frequency to
 * switch to post NOL.
 *
 */
QDF_STATUS ucfg_dfs_get_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
				       qdf_freq_t *postnol_cfreq2);
#else
static inline QDF_STATUS
ucfg_dfs_set_postnol_freq(struct wlan_objmgr_pdev *pdev,
			  qdf_freq_t postnol_freq)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_set_postnol_mode(struct wlan_objmgr_pdev *pdev,
			  uint16_t postnol_mode)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_set_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
			    qdf_freq_t postnol_cfreq2)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_get_postnol_freq(struct wlan_objmgr_pdev *pdev,
			  qdf_freq_t *postnol_freq)
{
	*postnol_freq = 0;
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_get_postnol_mode(struct wlan_objmgr_pdev *pdev,
			  uint8_t *postnol_mode)
{
	*postnol_mode = CH_WIDTH_INVALID;
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_get_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
			    qdf_freq_t *postnol_cfreq2)
{
	*postnol_cfreq2 = 0;
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * utils_dfs_reset_intercac() - Reset all params which decides
 *                              interCAC feature.
 * @pdev: Pointer to DFS pdev object.
 * Return: Nothing.
 */
void utils_dfs_reset_intercac(struct wlan_objmgr_pdev *pdev);
#else
static inline void
utils_dfs_reset_intercac(struct wlan_objmgr_pdev *pdev)
{
}
#endif

#ifdef QCA_DFS_BW_EXPAND
/**
 * utils_dfs_set_bw_expand_channel() - API to set user frequency and user
 *                                     configured phymode.
 * @pdev: Pointer to DFS pdev object.
 * @user_freq: frequency value configured by the user.
 * @user_mode: Phymode value configured by the user.
 *
 * Return: Success on storing user frequency and mode
 *         Failure on dfs object is null.
 */
QDF_STATUS utils_dfs_set_bw_expand_channel(struct wlan_objmgr_pdev *pdev,
					   qdf_freq_t user_freq,
					   enum wlan_phymode user_mode);

/**
 * ucfg_dfs_set_bw_expand() - Set the value of BW Expand feature.
 * @pdev: Pointer to DFS pdev object.
 * @bw_expand: Set BW Expand based on this value.
 *
 * Wrapper function for dfs_set_bw_expand() which enables/disables
 * the BW reduction feature.
 * This function is called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_dfs_set_bw_expand(struct wlan_objmgr_pdev *pdev,
		       bool bw_expand);

/**
 * ucfg_dfs_get_bw_expand() - Get the value of BW Expand feature.
 * @pdev: Pointer to DFS pdev object.
 * @bw_expand: Store the value of BW Expand feature
 *
 * Wrapper function for dfs_get_bw_expand() which displays value
 * of the BW reduction feature whether it is enabled or disabled.
 * This function is called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_dfs_get_bw_expand(struct wlan_objmgr_pdev *pdev,
		       bool *bw_expand);
#endif /* QCA_DFS_BW_EXPAND */

#ifdef QCA_DFS_BW_PUNCTURE
/**
 * ucfg_dfs_set_dfs_puncture() - Set the value of DFS puncturing feature.
 * @pdev: Pointer to DFS pdev object.
 * @is_dfs_punc_en: Set DFS puncturing based on this value.
 *
 * Wrapper function for dfs_set_dfs_puncture() which enables/disables
 * the DFS puncturing feature.
 * This function is called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_dfs_set_dfs_puncture(struct wlan_objmgr_pdev *pdev,
			  bool is_dfs_punc_en);

/**
 * ucfg_dfs_get_dfs_puncture() - Get the value of DFS puncturing feature.
 * @pdev: Pointer to DFS pdev object.
 * @is_dfs_punc_en: Store the value of DFS puncturing feature
 *
 * Wrapper function for dfs_get_dfs_puncture() which displays value
 * of the  DFS puncturing feature whether it is enabled or disabled.
 * This function is called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_dfs_get_dfs_puncture(struct wlan_objmgr_pdev *pdev,
			  bool *is_dfs_punc_en);
#endif /* QCA_DFS_BW_PUNCTURE */
