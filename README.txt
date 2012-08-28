Repo sructure.
	
	t2h\
		doc - containts 't2h' decumentation.
		src - containts 't2h' sources.
		toolchains - containts the gcc/cl/etc the cmake toolchains.
		packages - for the extra packages.
		bin - generate by bootstrap.[sh|bat], containts binaries files.
		lib - generate by bootstrap.[sh|bat], containts libraries files.

Supporting OS/Compilers.
	
	1.0 OS.
		Windows min. version XP with SP 1.
		Mac OS X min. version 10.6.5. 
	
	1.1 Compilers.
		1.1.1 Apple
			Min. version. apple llvm-gcc-4.2 
			Min. version. apple gcc-4.2
		1.1.2 Windows
			Min. version. Visual Studio 7.1 cl 
		1.1.3 Linux/BSD
			Min. version. gcc-4.2

Building 't2h'.

    2.0 Extra depends.
        The 't2h' library have several extra depends, boost[1.45 min. ver.], libtorrent[0.16.0 min. ver.],
        cmake the build system [2.8 min. ver.], Open SSL[as the boost and the libtorrent extra depends].
        To know how-to build/get libtorrent see the libtorrent[http://www.rasterbar.com/products/libtorrent] site.
        To know hot-tp build/get boost see the boost[www.boost.org] site.

    2.1 Mac OS X/Linux/BSD.
        To build 't2h' release dynamic library udner Linux/Mac OS X/BSD open 'terminal' -> cd to the repo root and type
        './bootstrap.sh -c --build-type=Release --shared=yes && make -C shared_build core'.
        For extra options see the './bootstrap.sh --help'.

	2.2 Windows.
		To build 't2h' release dynamic library under Windows platforms open cmd.exe, then setup Visual Studio envt., then cd to
		the repo root and type follow text './bootstrap.sh -c --build-type=Release --shared=yes && nmake core'
	
    2.3 [NOT IMPLEMENTED]
        To test 't2h', after building type to terminal/cmd.exe follow text 'cd shared_build && make/nmake check'

