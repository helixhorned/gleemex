FILES=gleemex.html

# NOTE: the 'source-highlight-args' attribute is not present in the default
# AsciiDoc distribution. It has to be hacked into its
# filters/source/source-highlight-filter.conf like this:
#
# filter="source-highlight -f xhtml -s {language} (...)"
#  -->
# filter="source-highlight {source-highlight-args} -f xhtml -s {language} (...)"
%.html: %.txt Makefile gleemex.style matlab.lang
	asciidoc -v -a source-highlight-args="--style-file=$(shell pwd)/gleemex.style" $<

all: $(FILES)

clean:
	rm -f $(FILES)
