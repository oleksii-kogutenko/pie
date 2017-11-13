CC := g++
CFLAGS := -g -O0

openssl_digest_builder_LIBS := -lssl -lcrypto
boost_filesystem_tests_LIBS := -lboost_system -lboost_filesystem

LDLIBS := $(boost_filesystem_tests_LIBS) $(openssl_digest_builder_LIBS)

all: boost_filesystem_tests openssl_digest_builder

boost_filesystem_tests: boost_filesystem_tests.o

openssl_digest_builder: openssl_digest_builder.o

