# github branches

The `main` branch is the default branch for this repository and is what is used for daily builds and all development. It should be stable enough for the daily builds on github to work. Feature branches and PRs will be merged into this branch.

If the current state of the `main` branch is considered to be stable, then the current wxUiEditor branch should be rebased from the main branch. The idea here is that the `main` branch can be used for development, and once it is considered sufficiently stable, then the wxUiEditor branch can effectively be a stable copy of the main branch.

# github workflows

Unfortunately, there does not appear to be a way to get gcc 11.4 installed on a github ubuntu-20-04 runner, and wxUiEditor cannot be built on an earlier gcc version. As a result, all Linux builds are done using ubuntu-22.04 which has gcc 12 installed. However, you can't run a Linux binary built on ubuntu-22.04 on ubuntu-20.04, because gcc will link to the shared version of glibc which is not compatible on the older OS. For releases, the build must be done on a private version of Ubuntu-20.04 with gcc 11.4.

At the time of this document, github does not have a fedora runner, so the fedora build must also be done on a private system.
