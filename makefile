src_tree := $(CURDIR)
build_tree := ${src_tree}/_build

.PHONY: prepare-%

all: build
	make -C ${build_tree} test

build: prepare
	make -C ${build_tree}

prepare: 
	mkdir -p ${build_tree}
	cd ${build_tree} && cmake -G"Unix Makefiles" $(src_tree)

clean:
	rm -rf ${build_tree}
