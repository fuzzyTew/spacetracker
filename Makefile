CXXFLAGS=-O3
LDFLAGS=-O3
TIME=$(shell date --iso=minutes)
launch: log_space
	time sudo ./log_space | sort -rn --temporary-directory="$$XDG_RUNTIME_DIR" | tee ${TIME}.log | head
