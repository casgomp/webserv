TESTERS = parsing server

all: $(TESTERS)

parsing:
	$(MAKE) -C parsing
	./parsing/run_parsing_tests

server:
	$(MAKE) -C server
	./server/run_server_tests

clean:
	@for dir in $(TESTERS); do $(MAKE) -C $$dir clean; done

fclean:
	@for dir in $(TESTERS); do $(MAKE) -C $$dir fclean; done

re: fclean all

.PHONY: all parsing server clean fclean re