:file_cabinet: MSSQL Backup Batch Restore Utility [![Build](https://github.com/jonaski/sqlrestore/workflows/build/badge.svg)](https://github.com/jonaski/sqlrestore/actions)
========================

SQL Restore is a batch restore program for MSSQL backups written in C++11.
It's using features in modern C++11, Boost and Qt.

![Browse](https://raw.githubusercontent.com/jonaski/sqlrestore/master/data/pictures/sqlrestore.gif)

### :heavy_check_mark: Features:

* Fully compatible with both SQL backups and zipped SQL backups.
* File view with search, sorting and FS listener with live updating.
* Uses threads.
* Uses libmagic to recognize files.
* SQL settings tester that runs in a concurrent thread.
* Looks for end of central directory signature before uncompressing ZIP files.
* Full CRC check of ZIP on restore.
* Works on Linux, macOS and Windows.
* Compatible with MSSQL 2008 R2 to SQL 2019 server on Linux and Windows.

The program is free software, released under GPL. If you like this program and can make use of it, consider sponsoring or donating.
To sponsor me visit [my GitHub sponsors profile](https://github.com/sponsors/jonaski)
Funding developers through GitHub Sponsors is one more way to contribute to open source projects you appreciate, it helps developers get the resources they need, and recognize contributors working behind the scenes to make open source better for everyone.
You can also make a one-time payment through [paypal.me/jonaskvinge](https://paypal.me/jonaskvinge)

### :heavy_exclamation_mark: Requirements

To build SQL Restore from source you need the following installed on your system:

* [CMake and Make tools](https://cmake.org/)
* [GCC](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/) compiler
* [POSIX thread (pthread)](http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html)
* [Boost](https://www.boost.org/)
* [Qt 5.9 or higher with components Core, Gui, Widgets, Network and Sql](https://www.qt.io/)
* [libmagic](http://darwinsys.com/file/)
* [zlib](https://www.zlib.net/)
* [quazip](https://github.com/stachenov/quazip)
* [FreeTDS](https://www.freetds.org/) or [Microsoft ODBC Driver for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server)


Install dependencies on openSUSE Leap 15.1:

    zypper addrepo https://download.opensuse.org/repositories/devel:libraries:c_c++/openSUSE_Leap_15.1/devel:libraries:c_c++.repo
    sudo zypper in cmake boost-devel file-devel libQt5Core-devel libQt5Gui-devel libQt5Widgets-devel libQt5Network-devel libQt5Sql-devel libquazip-qt5-devel libQt5Sql5-unixODBC

Install dependencies on Ubuntu:

    sudo apt install cmake libboost-dev libmagic-dev qtbase5-dev libqt5sql5-odbc libquazip5-dev

### :wrench: Compile and install:

    cd sqlrestore
    mkdir build && cd build
    cmake ..
    make -j$(nproc)
    sudo make install

### :wrench: Cross compile using MXE:

Shared:

MXE needs to be in $HOME/mxe-shared

Use the https://github.com/strawberrymusicplayer/strawberry-mxe repository.

    PKG_CONFIG_LIBDIR=$HOME/mxe-shared/usr/x86_64-w64-mingw32.shared/lib/pkgconfig cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32-shared.cmake -DARCH=x86_64
    make -j$(nproc)

### :open_file_folder: Copy dependencies for shared build:

    mkdir -p platforms imageformats sqldrivers styles
    cp $HOME/mxe-shared/usr/x86_64-w64-mingw32.shared/qt5/plugins/platforms/qwindows.dll platforms/
    cp $HOME/mxe-shared/usr/x86_64-w64-mingw32.shared/qt5/plugins/sqldrivers/qsqlodbc.dll sqldrivers/
    cp $HOME/mxe-shared/usr/x86_64-w64-mingw32.shared/qt5/plugins/styles/qwindowsvistastyle.dll styles/
    cp $HOME/mxe-shared/usr/x86_64-w64-mingw32.shared/qt5/plugins/imageformats/{qgif.dll,qico.dll,qjp2.dll,qjpeg.dll,qsvg.dll,qtiff.dll} imageformats/
    cp $HOME/mxe-shared/usr/x86_64-w64-mingw32.shared/bin/killproc.exe .
    $HOME/mxe-shared/tools/copydlldeps.sh -c -F . -R $HOME/mxe-shared/usr/x86_64-w64-mingw32.shared -d .

### :floppy_disk: Create nullsoft installer:

    cp ../data/{magic,magic.mgc} .
    cp ../dist/windows/{sqlrestore.ico,sqlrestore.nsi,*.nsh} .
    makensis sqlrestore.nsi

Static:

MXE needs to be in $HOME/mxe-static

Use the https://github.com/jonaski/mxe-qt repository.

    PKG_CONFIG_LIBDIR=$HOME/mxe-static/usr/x86_64-w64-mingw32.static/lib/pkgconfig cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain-mingw32-static.cmake
    make -j$(nproc)

### :penguin: unixODBC with FreeTDS

Add something like this to /etc/unixODBC/odbcinst.ini

    [FreeTDS]
    Description=FreeTDS
    Driver = /usr/lib64/libtdsodbc.so.0.0.0
    Setup = /usr/lib64/unixODBC/libtdsS.so

### :penguin: Install "ODBC Driver 17 for SQL Server" on openSUSE Leap 15.1:

    sudo zypper ar https://packages.microsoft.com/config/sles/15/prod.repo
    sudo ACCEPT_EULA=Y zypper in msodbcsql17 mssql-tools unixODBC-devel
    echo 'export PATH="$PATH:/opt/mssql-tools/bin"' >> ~/.bash_profile
    echo 'export PATH="$PATH:/opt/mssql-tools/bin"' >> ~/.bashrc

### :penguin: Install "ODBC Driver 17 for SQL Server" on Ubuntu 19.04:

    su
    apt install curl
    curl https://packages.microsoft.com/keys/microsoft.asc | apt-key add -
    curl https://packages.microsoft.com/config/ubuntu/19.04/prod.list > /etc/apt/sources.list.d/mssql-release.list
    exit
    sudo apt update
    sudo ACCEPT_EULA=Y apt install msodbcsql17 mssql-tools unixodbc-dev
    echo 'export PATH="$PATH:/opt/mssql-tools/bin"' >> ~/.bash_profile
    echo 'export PATH="$PATH:/opt/mssql-tools/bin"' >> ~/.bashrc
