:file_cabinet: SQL Restore [![Build Status](https://github.com/jonaski/sqlrestore/workflows/CI:%20Build%20Test/badge.svg)](https://github.com/jonaski/sqlrestore/actions)
========================

SQL Restore is a batch restore for MSSQL backups written in C++11.
It's using features in modern C++11, Boost and Qt.

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

### :heavy_exclamation_mark: Requirements

To build SQL Restore from source you need the following installed on your system:

* [CMake and Make tools](https://cmake.org/)
* [GCC](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/) compiler
* [POSIX thread (pthread)](http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html)
* [Boost](https://www.boost.org/)
* [Qt 5.9 or higher with components Core, Gui, Widgets, Network and Sql](https://www.qt.io/)
* [libmagic](http://darwinsys.com/file/)
* [Microsoft ODBC Driver for SQL Server](https://docs.microsoft.com/en-us/sql/connect/odbc/linux-mac/installing-the-microsoft-odbc-driver-for-sql-server)


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

MXE needs to be in $HOME/mxe

Shared:

    PKG_CONFIG_LIBDIR=$HOME/mxe/usr/x86_64-w64-mingw32.shared/lib/pkgconfig cmake .. -DCMAKE_TOOLCHAIN_FILE=../Toolchain-mingw32-shared.cmake -DARCH=x86_64 -DUSE_SYSTEM_SINGLEAPPLICATION=ON
    make -j$(nproc)

### :open_file_folder: Copy dependencies for shared build:

    mkdir -p platforms imageformats sqldrivers styles
    cp $HOME/mxe/usr/x86_64-w64-mingw32.shared/qt5/plugins/platforms/qwindows.dll platforms/
    cp $HOME/mxe/usr/x86_64-w64-mingw32.shared/qt5/plugins/sqldrivers/qsqlodbc.dll sqldrivers/
    cp $HOME/mxe/usr/x86_64-w64-mingw32.shared/qt5/plugins/styles/qwindowsvistastyle.dll styles/
    cp $HOME/mxe/usr/x86_64-w64-mingw32.shared/qt5/plugins/imageformats/{qgif.dll,qico.dll,qjp2.dll,qjpeg.dll,qsvg.dll,qtiff.dll} imageformats/
    cp $HOME/mxe/usr/x86_64-w64-mingw32.shared/bin/killproc.exe .
    $HOME/mxe/tools/copydlldeps.sh -c -F . -R $HOME/mxe/usr/x86_64-w64-mingw32.shared -d .

### :floppy_disk: Create nullsoft installer:

    cp ../data/{magic,magic.mgc} .
    cp ../dist/windows/{sqlrestore.ico,sqlrestore.nsi,*.nsh} .
    makensis sqlrestore.nsi

Static:

    export PATH=$HOME/mxe/usr/bin:$PATH
    mkdir build-win-static
    cd build-win-static
    $HOME/mxe/usr/x86_64-w64-mingw32.static/qt5/bin/qmake ../sqlrestore/sqlrestore.pro

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
