/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

package com.sanyinchen.jsbridge.load;

import android.content.Context;


/**
 * A class that stores JS bundle information and allows a {@link JSBundleLoaderDelegate}
 * (e.g. {@link CatalystInstance}) to load a correct bundle through {@link ReactBridge}.
 */
public abstract class JSBundleLoader {

    /**
     * This loader is recommended one for release version of your app. In that case local JS executor
     * should be used. JS bundle will be read from assets in native code to save on passing large
     * strings from java to native memory.
     */
    public static JSBundleLoader createAssetLoader(
            final Context context,
            final String assetUrl,
            final boolean loadSynchronously) {
        return new JSBundleLoader() {
            @Override
            public String loadScript(JSBundleLoaderDelegate delegate) {
                delegate.loadScriptFromAssets(context.getAssets(), assetUrl, loadSynchronously);
                return assetUrl;
            }
        };
    }


    /** Loads the script, returning the URL of the source it loaded. */
    public abstract String loadScript(JSBundleLoaderDelegate delegate);
}
