CXXFLAGS=-O3
LDFLAGS=-O3
TIME=$(shell date --iso=minutes)
launch: log_space
	time sudo ./log_space > ${TIME}.log
	sort -rn --temporary-directory="$$XDG_RUNTIME_DIR" ${TIME}.log > ${TIME}.sorted.log
	head ${TIME}.sorted.log | tee ${TIME}.summary
	git add ${TIME}.summary
	git commit -m ${TIME}
