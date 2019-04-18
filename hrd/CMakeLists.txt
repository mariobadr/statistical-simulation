# Compile the library that maintains an HRD profile.
add_subdirectory(model)

# Compile the executable that creates an HRD profile from a trace.
add_subdirectory(model-generator)

# Compile the executable that synthesizes a trace from an HRD profile.
add_subdirectory(trace-generator)
