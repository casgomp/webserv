# include <cerrno>
# include <csignal>
# include <cstdio>
# include <cstdlib>
# include <cstring>
# include <fstream>
# include <sstream>
# include <dirent.h>
# include <fcntl.h>
# include <poll.h>
# include <unistd.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <sys/wait.h>

# include "e2eHelpers.hpp"

/* =============================== Reply =============================== */

Reply::Reply() : status(0), complete(false), framed(false), closed(false) {}

std::string	Reply::header(const std::string &name) const
{
	std::map<std::string, std::string>::const_iterator	it = headers.find(e2e::lower(name));

	if (it == headers.end())
		return ("");
	return (it->second);
}

std::string	Reply::statusLine() const
{
	std::string	line = raw.substr(0, raw.find("\r\n"));

	if (line.empty())
		return ("<no response>");
	return (line);
}

namespace e2e
{

/* ============================== utilities ============================== */

std::string	toStr(long n)
{
	std::ostringstream	os;

	os << n;
	return (os.str());
}

bool	fileExists(const std::string &path)
{
	struct stat	st;

	return (stat(path.c_str(), &st) == 0);
}

bool	isDir(const std::string &path)
{
	struct stat	st;

	return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

bool	mkdirs(const std::string &path)
{
	std::string	cur;

	for (size_t i = 0; i <= path.size(); i++)
	{
		if (i == path.size() || path[i] == '/')
		{
			if (!cur.empty() && !isDir(cur) && mkdir(cur.c_str(), 0755) != 0)
				return (false);
		}
		if (i < path.size())
			cur += path[i];
	}
	return (true);
}

bool	writeFile(const std::string &path, const std::string &content)
{
	std::ofstream	out(path.c_str(), std::ios::binary);

	if (!out.is_open())
		return (false);
	out.write(content.data(), content.size());
	return (out.good());
}

std::string	readFile(const std::string &path)
{
	std::ifstream		in(path.c_str(), std::ios::binary);
	std::ostringstream	os;

	if (!in.is_open())
		return ("");
	os << in.rdbuf();
	return (os.str());
}

void	removeTree(const std::string &path)
{
	struct stat	st;

	if (lstat(path.c_str(), &st) != 0)
		return ;
	if (S_ISDIR(st.st_mode))
	{
		chmod(path.c_str(), 0755);
		DIR	*dir = opendir(path.c_str());
		if (dir)
		{
			struct dirent	*e;
			while ((e = readdir(dir)) != NULL)
			{
				std::string	name = e->d_name;
				if (name != "." && name != "..")
					removeTree(path + "/" + name);
			}
			closedir(dir);
		}
		rmdir(path.c_str());
	}
	else
		unlink(path.c_str());
}

bool	startsWith(const std::string &s, const std::string &prefix)
{
	return (s.compare(0, prefix.size(), prefix) == 0);
}

bool	contains(const std::string &s, const std::string &needle)
{
	return (s.find(needle) != std::string::npos);
}

std::string	lower(const std::string &s)
{
	std::string	out = s;

	for (size_t i = 0; i < out.size(); i++)
		if (out[i] >= 'A' && out[i] <= 'Z')
			out[i] = out[i] - 'A' + 'a';
	return (out);
}

/* ========================== request builders ========================== */

std::string	raw(const std::string &method, const std::string &target,
				const std::string &version, const std::string &headers)
{
	return (method + " " + target + " " + version + "\r\n" + headers + "\r\n");
}

std::string	get(const std::string &target, const std::string &extraHeaders)
{
	return (raw("GET", target, "HTTP/1.1", "Host: localhost\r\n" + extraHeaders));
}

std::string	head(const std::string &target)
{
	return (raw("HEAD", target, "HTTP/1.1", "Host: localhost\r\n"));
}

std::string	del(const std::string &target)
{
	return (raw("DELETE", target, "HTTP/1.1", "Host: localhost\r\n"));
}

std::string	post(const std::string &target, const std::string &body,
				const std::string &contentType, const std::string &extraHeaders)
{
	return (raw("POST", target, "HTTP/1.1", "Host: localhost\r\n"
		"Content-Type: " + contentType + "\r\n"
		"Content-Length: " + toStr(body.size()) + "\r\n" + extraHeaders) + body);
}

std::string	postChunked(const std::string &target, const std::string &encodedBody,
				const std::string &extraHeaders)
{
	return (raw("POST", target, "HTTP/1.1", "Host: localhost\r\n"
		"Content-Type: text/plain\r\n"
		"Transfer-Encoding: chunked\r\n" + extraHeaders) + encodedBody);
}

std::string	chunk(const std::string &data)
{
	std::ostringstream	os;

	os << std::hex << data.size();
	return (os.str() + "\r\n" + data + "\r\n");
}

/* ================================ Conn ================================ */

static int	openSocket(int port)
{
	struct sockaddr_in	addr;
	int					fd = socket(AF_INET, SOCK_STREAM, 0);
	struct timeval		tv;

	if (fd < 0)
		return (-1);
	std::memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	if (connect(fd, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0)
	{
		close(fd);
		return (-1);
	}
	tv.tv_sec = 3;						// never block forever in send()
	tv.tv_usec = 0;
	setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
	return (fd);
}

Conn::Conn(int port) : _fd(openSocket(port)) {}

Conn::~Conn()
{
	if (_fd >= 0)
		close(_fd);
}

bool	Conn::connected() const
{
	return (_fd >= 0);
}

bool	Conn::send(const std::string &data)
{
	size_t	sent = 0;

	if (_fd < 0)
		return (false);
	while (sent < data.size())
	{
		ssize_t	n = ::send(_fd, data.c_str() + sent, data.size() - sent, MSG_NOSIGNAL);
		if (n < 0 && errno == EINTR)
			continue ;
		if (n <= 0)
			return (false);
		sent += n;
	}
	return (true);
}

int	Conn::fill(int timeoutMs)
{
	struct pollfd	p;
	char			buf[65536];
	ssize_t			n;
	int				r;

	if (_fd < 0)
		return (0);
	p.fd = _fd;
	p.events = POLLIN;
	p.revents = 0;
	r = poll(&p, 1, timeoutMs);
	if (r == 0)
		return (-1);
	if (r < 0)
		return (errno == EINTR ? -1 : 0);
	n = recv(_fd, buf, sizeof(buf), 0);
	if (n > 0)
	{
		_buf.append(buf, n);
		return (1);
	}
	if (n < 0 && (errno == EINTR || errno == EAGAIN))
		return (-1);
	return (0);
}

bool	Conn::silent(int ms)
{
	if (!_buf.empty())
		return (false);
	return (fill(ms) == -1);
}

bool	Conn::peerClosed(int waitMs)
{
	if (!_buf.empty())
		return (false);
	return (fill(waitMs) == 0);
}

// decodes a chunked response body from _buf (already past the headers)
bool	Conn::readChunkedBody(std::string &out, int timeoutMs)
{
	for (;;)
	{
		size_t	eol;
		while ((eol = _buf.find("\r\n")) == std::string::npos)
			if (fill(timeoutMs) <= 0)
				return (false);
		size_t	size = std::strtoul(_buf.substr(0, eol).c_str(), NULL, 16);
		_buf.erase(0, eol + 2);
		while (_buf.size() < size + 2)
			if (fill(timeoutMs) <= 0)
				return (false);
		out += _buf.substr(0, size);
		_buf.erase(0, size + 2);
		if (size == 0)
			return (true);
	}
}

Reply	Conn::read(bool isHead, int timeoutMs)
{
	Reply		r;
	size_t		end;
	int			st;

	while ((end = _buf.find("\r\n\r\n")) == std::string::npos)
	{
		st = fill(timeoutMs);
		if (st <= 0)
		{
			r.raw = _buf;
			r.closed = (st == 0);
			return (r);
		}
	}

	std::string			head = _buf.substr(0, end);
	std::istringstream	lines(head);
	std::string			line;

	std::getline(lines, line);
	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	{
		size_t	sp1 = line.find(' ');
		size_t	sp2 = (sp1 == std::string::npos) ? sp1 : line.find(' ', sp1 + 1);

		if (sp1 != std::string::npos)
		{
			r.version = line.substr(0, sp1);
			r.status = std::atoi(line.c_str() + sp1 + 1);
			if (sp2 != std::string::npos)
				r.reason = line.substr(sp2 + 1);
		}
	}
	while (std::getline(lines, line))
	{
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t	colon = line.find(':');
		if (colon == std::string::npos)
			continue ;
		std::string	value = line.substr(colon + 1);
		size_t		b = value.find_first_not_of(" \t");
		r.headers[lower(line.substr(0, colon))] = (b == std::string::npos) ? "" : value.substr(b);
	}

	size_t	bodyStart = end + 4;
	size_t	len = 0;

	if (isHead || r.status == 204 || r.status == 304 || r.status / 100 == 1)
		r.framed = true;										// no body by definition
	else if (lower(r.header("transfer-encoding")) == "chunked")
	{
		_buf.erase(0, bodyStart);
		r.framed = true;
		r.complete = readChunkedBody(r.body, timeoutMs);
		r.raw = head + "\r\n\r\n" + r.body;
		return (r);
	}
	else if (r.headers.find("content-length") != r.headers.end())
	{
		r.framed = true;
		len = std::strtoul(r.header("content-length").c_str(), NULL, 10);
	}

	while (_buf.size() < bodyStart + len)
	{
		st = fill(timeoutMs);
		if (st <= 0)
		{
			r.raw = _buf;
			r.body = _buf.substr(bodyStart);
			r.closed = (st == 0);
			_buf.clear();
			return (r);
		}
	}
	r.body = _buf.substr(bodyStart, len);
	r.raw = _buf.substr(0, bodyStart + len);
	_buf.erase(0, bodyStart + len);
	r.complete = true;
	return (r);
}

Reply	exchange(const std::string &request, bool isHead, bool detectClose, int port)
{
	Conn	c(port);
	Reply	r;

	if (!c.connected())
		return (r);
	c.send(request);			// the server may hang up before reading everything; that is fine
	r = c.read(isHead);
	if (detectClose && r.status != 0)
		r.closed = c.peerClosed(r.header("connection") == "close" ? 1000 : 150);
	return (r);
}

/* ============================== ServerProc ============================== */

ServerProc::ServerProc() : _pid(-1), _code(-1) {}

ServerProc::~ServerProc()
{
	stop();
}

pid_t	ServerProc::pid() const
{
	return (_pid);
}

int	ServerProc::lastExit() const
{
	return (_code);
}

void	ServerProc::reap()
{
	int	status = 0;

	if (_pid > 0 && waitpid(_pid, &status, WNOHANG) == _pid)
	{
		_code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
		_pid = -1;
	}
}

bool	ServerProc::running()
{
	reap();
	return (_pid > 0);
}

int	ServerProc::waitExit(int ms)
{
	for (int waited = 0; waited <= ms; waited += 10)
	{
		reap();
		if (_pid < 0)
			return (_code);
		usleep(10000);
	}
	return (-1);
}

bool	ServerProc::start(const std::string &conf, int port, const std::string &log)
{
	_code = -1;
	_pid = fork();
	if (_pid < 0)
		return (false);
	if (_pid == 0)
	{
		int	fd = open(log.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd >= 0)
		{
			dup2(fd, STDOUT_FILENO);
			dup2(fd, STDERR_FILENO);
			close(fd);
		}
		execl(E2E_BIN, E2E_BIN, conf.c_str(), static_cast<char *>(0));
		::_exit(127);
	}
	if (port <= 0)
		return (true);
	for (int i = 0; i < 50; i++)
	{
		Conn	probe(port);

		if (probe.connected())
			return (true);
		reap();
		if (_pid < 0)								// died while starting
			return (false);
		usleep(100000);
	}
	return (false);
}

bool	ServerProc::stop()
{
	bool	clean = true;

	if (_pid > 0)
	{
		kill(_pid, SIGINT);
		if (waitExit(2000) == -1)
		{
			clean = false;
			kill(_pid, SIGKILL);
			waitpid(_pid, NULL, 0);
			_pid = -1;
		}
	}
	return (clean);
}

void	countChildren(pid_t pid, int &children, int &zombies)
{
	DIR	*proc = opendir("/proc");

	children = 0;
	zombies = 0;
	if (!proc)
		return ;
	struct dirent	*e;
	while ((e = readdir(proc)) != NULL)
	{
		if (e->d_name[0] < '0' || e->d_name[0] > '9')
			continue ;
		std::string	stat = readFile(std::string("/proc/") + e->d_name + "/stat");
		size_t		rp = stat.rfind(')');
		if (rp == std::string::npos || rp + 4 >= stat.size())
			continue ;
		char		state = stat[rp + 2];
		long		ppid = std::strtol(stat.c_str() + rp + 4, NULL, 10);
		if (ppid == pid)
		{
			children++;
			if (state == 'Z')
				zombies++;
		}
	}
	closedir(proc);
}

}	// namespace e2e
