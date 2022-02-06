#include <iostream>
#include <string>
#include <iomanip>

class ProgressBar
{
private:
	size_t m_len = 50;
	char m_fill = '#';
	double m_last = 0;
	bool m_init = false;

public:
	void setFillType(const char c)
	{
		m_fill = c;
	}

	void setLength(const size_t len)
	{
		m_len = len;
	}

	void update(const double pos)
	{
		if (!m_init || pos - m_last > 0.001)
		{
			int cnt = static_cast<int>(pos * m_len);
			std::cout << "\r[" << std::string(cnt, m_fill) << std::string(m_len - cnt, ' ') << "] ";
			std::cout << std::fixed << std::setprecision(2) << pos * 100 << "%";
			m_last = pos;
		}

		m_init = true;
	}

	void finish()
	{
		std::cout << "\r[" << std::string(m_len, m_fill) << "] 100.0%" << std::endl;
	}
};