FILES=gleemex.html

# Notes:
#
# 1. The 'source-highlight-args' attribute is not present in the default
# AsciiDoc distribution. It has to be hacked into its
# filters/source/source-highlight-filter.conf like this:
#
# filter="source-highlight -f xhtml -s {language} (...)"
#  -->
# filter="source-highlight {source-highlight-args} -f xhtml -s {language} (...)"
#
# 2. Also source-highlight must be told how that matlab.lang is the language file for MATLAB

all: $(FILES)

file1=/usr/local/etc/asciidoc/filters/source/source-highlight-filter.conf
file2=/usr/share/source-highlight/lang.map
file3=/usr/share/source-highlight/matlab.lang

check-required-modifications:
	@grep -q "source-highlight {source-highlight-args}" $(file1) || (echo "ERROR: Must edit $(file1)" && false)
	@grep -q "matlab = matlab.lang" $(file2) || (echo "ERROR: Must edit $(file2)" && false)
	@test -e $(file3) || (echo "ERROR: must copy matlab.lang to $(file3)" && false)
.PHONY: check-required-modifications

%.html: %.txt check-required-modifications Makefile gleemex.style matlab.lang
	asciidoc -v -a source-highlight-args="--style-file=$(shell pwd)/gleemex.style" $<

clean:
	rm -f $(FILES)
