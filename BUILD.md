How to Build `ipc`
==================

To produce `libipc.a` you will need:

- an installation of the BDE version of [waf][waf] in your path
- a clone of the open source [bde][bde] repository
- a clone of the open source [bde-tools][bde-tools] repository
- an installed version of [gtest][gtest] visible to [Pkg-config][pkg-config]
- a clone of this repository

Then the build proceeds as [documented][bde-doc] using a waf "workspace." What
follows is an example.

Example Build
-------------
```
$ mkdir -p $TMPDIR/$USER/workspace && cd "$_"

$ ln -s /path/to/my/bde .

$ ln -s /path/to/my/ipc .

$ cp /path/to/my/bde-tools/share/wscript .

$ waf configure >/dev/null

$ waf build >/dev/null

$ ls build/ipc/src/groups/ipc/libipc.a 
build/ipc/src/groups/ipc/libipc.a
```

[pkg-config]: https://bloomberg.github.io/bde-tools/waf.html#handling-external-dependencies-using-pkg-config
[gtest]: https://github.com/google/googletest
[waf]: https://bloomberg.github.io/bde-tools/tutorials.html#use-waf-to-build-bde
[bde]: https://github.com/bloomberg/bde
[bde-tools]: https://github.com/bloomberg/bde-tools
[bde-doc]: http://bloomberg.github.io/bde-tools/tutorials.html#use-waf-workspace-to-build-multiple-bde-style-repositories
