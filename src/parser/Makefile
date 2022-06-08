ANTLR=java -cp "$(TOP_PATH)/lib/antlr-4.9.1-complete.jar:$(CLASSPATH)" \
		org.antlr.v4.Tool -visitor

LEXGRAMMAR=DecafLexer.g4
GRAMMAR=DecafParser.g4

CXXARGS+= -I$(ANTLR_OUTPUT)
export

ANTLR_SRCS+=$(ANTLR_OUTPUT)/DecafLexer.cpp\
			  $(ANTLR_OUTPUT)/DecafParserBaseListener.cpp\
			  $(ANTLR_OUTPUT)/DecafParserListener.cpp\
			  $(ANTLR_OUTPUT)/DecafParserParser.cpp\
			  $(ANTLR_OUTPUT)/DecafParserBaseVisitor.cpp\
			  $(ANTLR_OUTPUT)/DecafParserVisitor.cpp
ANTLR_OBJS=$(ANTLR_SRCS:.cpp=.o)


all: parser CommonLexer.o ASTPrinter.o $(ANTLR_OBJS)
	cp *.o $(OUTPUT)
	cp $(ANTLR_OUTPUT)/*.o $(OUTPUT)

$(ANTLR_SRCS): $(GRAMMAR) $(LEXGRAMMAR)
	mkdir -p $(ANTLR_OUTPUT)
	$(ANTLR) $(LEXGRAMMAR) -Dlanguage=Cpp -o $(ANTLR_OUTPUT)
	$(ANTLR) $(GRAMMAR) -Dlanguage=Cpp -o $(ANTLR_OUTPUT)

$(ANTLR_OBJS): $(ANTLR_SRCS)
	$(CXX) $(CXXARGS) -w $(patsubst %.o,%.cpp,$@) -o $@


.PHONY: parser
parser: $(ANTLR_SRCS) $(ANTLR_OBJS)

CommonLexer.o: CommonLexer.cpp CommonLexer.h
	$(CXX) $(CXXARGS) $< -o $@

ASTPrinter.o: ASTPrinter.cpp ASTPrinter.h
	$(CXX) $(CXXARGS) $< -o $@

clean:
	rm -rf $(ANTLR_OUTPUT)
	rm -rf *.o