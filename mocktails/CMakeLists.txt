# Compile the library that maintains an Mocktails profile.
add_subdirectory(model)

# Compile the executable that creates an Mocktails profile from a trace.
add_subdirectory(model-generator)

# Compile the library that converts a Mocktails profile into a human-readable format.
add_subdirectory(model-dumper)

# Compile the executable that synthesizes a trace from an Mocktails profile.
add_subdirectory(trace-generator)
