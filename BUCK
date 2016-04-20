cxx_library(
  name = 'Rename',
  header_namespace = 'Rename',
  srcs = [
    'NodeOptions.cpp',
    'Nodes.cpp'
  ],
  exported_headers = [
    'Nodes.h',
    'Matchers.h',
    'Options.h',
    'Utility.h',
    'Handlers.h'
  ],
  visibility=['PUBLIC']
)

cxx_binary(
  name = 'rn',
  srcs = [ 'Rename.cpp' ],
  deps = [ ':Rename' ]
)
