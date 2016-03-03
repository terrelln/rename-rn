# rename-rn

This tool is still a work in progress.
Editor plugins to come.

## Potential Problems

* Typedefs in templates
* Macros
* Using shadows / function overloads

## Command Line

The command line args need to be redone.

```
./rn <file.cpp> -new-name=<name> -line=<line> -column=<column> -rewrite=<true/false> [ -- <flags for compiler> ]
```

You don't need the `-- <flags>` if there is a `compile_commands.json` in any parent directory that specifies how to compile the file.
