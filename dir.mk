IMPORT_CPPFLAGS += $(patsubst %,-I%/tools/tcp,$(IMPORT_TREES))
IMPORT_CPPFLAGS += $(patsubst %,-I%/tools/cpkg,$(IMPORT_TREES))
IMPORT_LIBRARY_FLAGS += $(patsubst %,-L%/tools/tcp,$(IMPORT_TREES))
IMPORT_LIBRARY_FLAGS += $(patsubst %,-L%/tools/cpkg,$(IMPORT_TREES))

LIBS = -lssh2
TARGET = test
OBJ = main.o SshClient.o

all:: $(TARGET) 

$(TARGET): $(OBJ)
	@(libs="$(LIBS)"; $(CXXExecutable))

