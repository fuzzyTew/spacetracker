CXXFLAGS=-O3
LDFLAGS=-O3
TIME=$(shell date --iso=minutes)
launch: log_space
	time sudo ./log_space > ${TIME}.traversal.log
	sort -rn --temporary-directory="$$XDG_RUNTIME_DIR" ${TIME}.traversal.log > ${TIME}.resorted.log
	head ${TIME}.resorted.log | tee ${TIME}.summary
	git add ${TIME}.summary
	git commit -m ${TIME}
