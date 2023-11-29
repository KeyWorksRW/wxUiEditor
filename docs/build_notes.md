# github branches

The `main` branch is the default branch for this repository and is what is used for daily builds and all development. It should be stable enough for the daily builds on github to work. Feature branches and PRs will be merged into this branch.

The `wxUiEditor_1_2` branch is the current stable branch for the 1.2.x series of releases. This branch will _never_ be merged back into the `main` branch. Instead commits in the main branch that are appropriate for the 1.2.x series will be cherry-picked into this branch. When the branch is ready for release, it will be built and released from this branch.

# github workflows

Unfortunately, I have not been able to get gcc 11.4 installed on a github ubuntu-20-04 runner, and wxUiEditor will not build on an earlier gcc version. As a result, all Linux builds are done using ubuntu-22.04 which has gcc 12 installed. However, you can't run a Linux binary built on ubuntu-22.04 on ubuntu-20.04, because gcc will link to the shared version of glibc which is not compatible on the older OS. For releases, the build must be done on a private version of Ubuntu-20.04 with gcc 11.4.

At the time of this document, github does not have a fedora runner, so the fedora build must also be done on a private system.
