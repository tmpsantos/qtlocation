TEMPLATE = subdirs

SUBDIRS = nokia mapbox esri itemsoverlay

qtConfig(concurrent) {
    SUBDIRS += osm
}

qtConfig(opengl):qtConfig(c++14):!win32|mingw:!qnx {
    !exists(../../3rdparty/mapbox-gl-native/mapbox-gl-native.pro) {
        warning("Submodule mapbox-gl-native does not exist. Run 'git submodule update --init' on qtlocation.")
    } else {
        SUBDIRS += mapboxgl ../../3rdparty/mapbox-gl-native
        mapboxgl.depends = ../../3rdparty/mapbox-gl-native
    }
}
