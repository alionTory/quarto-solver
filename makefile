OPTIONS = -O3 -std=c++17
OBJDIR = obj
SRCDIR = src

OBJS = $(OBJDIR)/main.o \
       $(OBJDIR)/Board.o \
       $(OBJDIR)/negamax.o \
       $(OBJDIR)/MonteCarlo.o

all: $(OBJS)
	g++ $(OPTIONS) -o QuartoCppCode.out $(OBJS)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp | $(OBJDIR)
	g++ $(OPTIONS) -c $(SRCDIR)/main.cpp -o $(OBJDIR)/main.o

$(OBJDIR)/Board.o: $(SRCDIR)/Board.cpp | $(OBJDIR)
	g++ $(OPTIONS) -c $(SRCDIR)/Board.cpp -o $(OBJDIR)/Board.o

$(OBJDIR)/negamax.o: $(SRCDIR)/negamax.cpp | $(OBJDIR)
	g++ $(OPTIONS) -c $(SRCDIR)/negamax.cpp -o $(OBJDIR)/negamax.o

$(OBJDIR)/MonteCarlo.o: $(SRCDIR)/MonteCarlo.cpp | $(OBJDIR)
	g++ $(OPTIONS) -c $(SRCDIR)/MonteCarlo.cpp -o $(OBJDIR)/MonteCarlo.o

clean:
	rm -rf $(OBJDIR) QuartoCppCode.out
