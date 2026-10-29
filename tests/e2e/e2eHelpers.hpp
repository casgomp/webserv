#ifndef E2E_HELPERS_HPP
# define E2E_HELPERS_HPP

# include <map>
# include <string>
# include <sys/types.h>

// Paths are relative to tests/e2e (the Makefile runs the tester from there).
# define E2E_BIN	"../../webserv"
# define E2E_PORT	18080

struct Reply
{
	Reply();
	int									status;		// 0 = no (parsable) response
	std::string							version;
	std::string							reason;
	std::map<std::string, std::string>	headers;	// names lower-cased
	std::string							body;
	std::string							raw;
	bool								complete;	// whole response (headers + body) received
	bool								framed;		// had Content-Length (or no body by definition)
	bool								closed;		// peer closed the connection (only if asked)
	std::string							header(const std::string &name) const;
	std::string							statusLine() const;
};

namespace e2e
{
	/* ---- small utilities ---- */
	std::string	toStr(long n);
	bool		fileExists(const std::string &path);
	bool		isDir(const std::string &path);
	bool		mkdirs(const std::string &path);
	bool		writeFile(const std::string &path, const std::string &content);
	std::string	readFile(const std::string &path);
	void		removeTree(const std::string &path);
	bool		startsWith(const std::string &s, const std::string &prefix);
	bool		contains(const std::string &s, const std::string &needle);
	std::string	lower(const std::string &s);

	/* ---- request builders (everything is plain HTTP/1.1 text) ---- */
	std::string	get(const std::string &target, const std::string &extraHeaders = "");
	std::string	head(const std::string &target);
	std::string	del(const std::string &target);
	std::string	post(const std::string &target, const std::string &body,
					const std::string &contentType = "text/plain",
					const std::string &extraHeaders = "");
	std::string	postChunked(const std::string &target, const std::string &encodedBody,
					const std::string &extraHeaders = "");
	std::string	chunk(const std::string &data);			// "<hex>\r\n<data>\r\n"
	std::string	raw(const std::string &method, const std::string &target,
					const std::string &version, const std::string &headers = "");

	/* ---- one TCP connection to the server under test ---- */
	class Conn
	{
	public:
		Conn(int port = E2E_PORT);
		~Conn();
		bool	connected() const;
		bool	send(const std::string &data);			// false once the peer is gone
		Reply	read(bool isHead = false, int timeoutMs = 3000);	// exactly ONE response
		bool	silent(int ms);							// nothing arrived within ms
		bool	peerClosed(int waitMs);					// EOF / reset seen within waitMs
	private:
		Conn(const Conn &);
		Conn	&operator=(const Conn &);
		int		fill(int timeoutMs);					// 1 data, 0 EOF/reset, -1 timeout
		bool	readChunkedBody(std::string &out, int timeoutMs);
		int			_fd;
		std::string	_buf;
	};

	// new connection, one request, one response. detectClose also looks for EOF.
	Reply	exchange(const std::string &request, bool isHead = false,
				bool detectClose = false, int port = E2E_PORT);

	/* ---- the webserv process under test ---- */
	class ServerProc
	{
	public:
		ServerProc();
		~ServerProc();
		bool	start(const std::string &conf, int port, const std::string &log);
		bool	running();									// false once it exited or crashed
		int		waitExit(int ms);							// exit code, 128+sig if killed, -1 still running
		bool	stop();										// SIGINT; true if it exited by itself within 2 s
		pid_t	pid() const;
		int		lastExit() const;							// code of the last exit (see waitExit)
	private:
		pid_t	_pid;
		int		_code;
		void	reap();
	};

	// children of pid; zombies is how many of them are defunct (Linux /proc)
	void	countChildren(pid_t pid, int &children, int &zombies);
}

#endif
