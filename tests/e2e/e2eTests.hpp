#ifndef E2E_TESTS_HPP
# define E2E_TESTS_HPP

# include <string>
# include "../TestSuite.hpp"
# include "e2eHelpers.hpp"

/*
** End-to-end tests: start the real ./webserv, talk to it over real sockets.
**
** Check messages start with the unit-test file the case comes from:
**   [P] HttpRequestParserTests   [C] ConfigParserTests
**   [S] ServerTests              [R] HttpResponseTests
**   [T] TESTING.md only (not covered by a unit test)
*/
class e2eTests : public TestSuite {
public:
	e2eTests();

	void	run_all();

private:
	e2e::ServerProc	_server;
	bool			_isRoot;		// root ignores file permissions: those checks are skipped
	bool			_haveCgi;		// /usr/bin/python3 present

	/* ---- infrastructure (e2eTests.cpp) ---- */
	bool	startMainServer();
	void	setupFixtures();
	void	cleanupFixtures();
	void	section(const std::string &title);
	void	alive(const std::string &after);
	void	expect(const std::string &req, int status, const std::string &msg);
	void	expectOpen(const std::string &req, int status, const std::string &msg);
	void	expectClosed(const std::string &req, int status, const std::string &msg);
	void	expectNot(const std::string &req, int status, const std::string &msg);
	void	dribble(const std::string &first, const std::string &second,
				int status, const std::string &msg);

	/* ---- 1-5: HTTP protocol (e2eTestsProtocol.cpp) ---- */
	void	test_responseFormat();
	void	test_requestLine();
	void	test_headersAndFraming();
	void	test_body();
	void	test_connection();

	/* ---- 6-10: server behaviour (e2eTestsServer.cpp) ---- */
	void	test_methods();
	void	test_pathsAndStaticFiles();
	void	test_routing();
	void	test_redirectsAutoindexIndex();
	void	test_cgi();

	/* ---- 11: configuration (e2eTestsConfig.cpp) ---- */
	void	test_config();
};

#endif
