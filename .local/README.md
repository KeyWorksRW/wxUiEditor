When copilot invokes --verify to ensure that _no_ changes have been made to code generation, it will assume you have done the following from the command line in the same directory as this README.md file:

```
git clone --depth 1 --no-single-branch --recurse-submodules --shallow-submodules https://github.com/KeyWorksRW/wxUiEditor_tests.git
```

You may also want to clone in your own version of a repository that you can use to test actual code generation changes. For example, the maintainter has a private repository which was added via:

```
git clone --depth 1 --no-single-branch https://github.com/KeyWorksRW/quick.git
```
