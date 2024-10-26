#include <QApplication>

#include "converter.h"
#include <iostream>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Check if we have enough arguments.
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <raster_path> <shp_path> <out_dir>" << std::endl;
        return 1;
    }

    // Assign the file paths from the command line arguments.
    auto raster_path = argv[1];
    auto shp_path = argv[2];
    auto out_dir = argv[3];
    auto tileSize = QSize(1024, 1024);

    // Call the function with the command line arguments.
    GisConverter::getInstance().cutIntoTiles(
        raster_path,
        shp_path,
        tileSize,
        out_dir
    );

    return 0;
}
