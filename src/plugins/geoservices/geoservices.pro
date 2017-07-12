TEMPLATE = subdirs

SUBDIRS = nokia mapbox esri itemsoverlay

qtConfig(concurrent) {
    SUBDIRS += osm
}

qtConfig(opengl):qtConfig(c++14):!win32|mingw:!qnx {
    SUBDIRS += mapboxgl ../../3rdparty/mapboxgl
    mapboxgl.depends = ../../3rdparty/mapboxgl
}
