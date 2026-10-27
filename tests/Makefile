TESTERS = parsing server response e2e

all: $(TESTERS)

parsing:
	$(MAKE) -C parsing
	./parsing/run_parsing_tests

server:
	$(MAKE) -C server
	./server/run_server_tests

response:
	$(MAKE) -C response
	./response/run_response_tests

e2e:
	$(MAKE) -C e2e
	cd e2e && ./run_e2e_tests

clean:
	@for dir in $(TESTERS); do $(MAKE) -C $$dir clean; done

fclean:
	@for dir in $(TESTERS); do $(MAKE) -C $$dir fclean; done

re: fclean all

.PHONY: all response parsing server e2e clean fclean re