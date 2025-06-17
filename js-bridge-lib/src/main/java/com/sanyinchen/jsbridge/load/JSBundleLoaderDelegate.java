/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.load;

import android.content.Context;
import android.content.res.AssetManager;

/**
 * An interface for classes that initialize JavaScript using {@link JSBundleLoader}
 */
public interface JSBundleLoaderDelegate {

    /**
     * Load a JS bundle from Android assets. See {@link JSBundleLoader#createAssetLoader(Context, String, boolean)}
     * @param assetManager
     * @param assetURL
     * @param loadSynchronously
     */
    void loadScriptFromAssets(AssetManager assetManager, String assetURL, boolean loadSynchronously);
}
