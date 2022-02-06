#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "ProgressBar.h"

enum H265NALUnitType
{
	NAL_TRAIL_N = 0, // ��IDR��Ƶ֡
	NAL_TRAIL_R = 1,
	NAL_TSA_N = 2,
	NAL_TSA_R = 3,
	NAL_STSA_N = 4,
	NAL_STSA_R = 5,
	NAL_RADL_N = 6,
	NAL_RADL_R = 7,
	NAL_RASL_N = 8,
	NAL_RASL_R = 9,
	NAL_BLA_W_LP = 16,
	NAL_BLA_W_RADL = 17,
	NAL_BLA_N_LP = 18,
	NAL_IDR_W_RADL = 19,  // IDR
	NAL_IDR_N_LP = 20,
	NAL_CRA_NUT = 21,
	NAL_VPS = 32,  // vps
	NAL_SPS = 33,  // sps
	NAL_PPS = 34,  // pps
	NAL_AUD = 35,
	NAL_EOS_NUT = 36,
	NAL_EOB_NUT = 37,
	NAL_FD_NUT = 38,
	NAL_SEI_PREFIX = 39,
	NAL_SEI_SUFFIX = 40,
};

struct H265Nalu
{
	std::uint64_t pos = 0;  // �ļ�λ��
	int naluType = 0;       // Nalu��Ԫ����
};

bool isH265NaluHead(unsigned char* dat)
{
	if (0x00 == dat[0] && 0x00 == dat[1] && 0x00 == dat[2] && 0x01 == dat[3])
	{
		return true;
	}
	return false;
}

// ����H265 Naluͷλ�ú�����
std::vector<H265Nalu> findH265NaluHead(const std::string& fileName)
{
	std::vector<H265Nalu> vecNaluHead;  // Naluͷλ�ú�Nalu����

	unsigned char dat[5] = { 0 };
	std::uint64_t pos = 0;

	std::ifstream in(fileName, std::ios::binary);
	in.seekg(0, std::ios::end);
	std::uint64_t len = in.tellg();
	in.seekg(0, std::ios::beg);

	in.read(reinterpret_cast<char*>(dat), 5); // �ȶ�ȡ5�ֽ�
	pos = 4;

	if (isH265NaluHead(dat))
	{
		H265Nalu nalu;
		nalu.pos = 0;
		nalu.naluType = (dat[4] & 0x7e) >> 1;
		vecNaluHead.push_back(nalu);
	}

	ProgressBar bar;
	//�����ļ���Ѱ��Naluͷ
	while (true)
	{
		dat[0] = dat[1];
		dat[1] = dat[2];
		dat[2] = dat[3];
		dat[3] = dat[4];
		in.read(reinterpret_cast<char*>(dat + 4), 1);

		if (in.eof())
		{
			break;
		}
		else
		{
			pos++;
		}

		if (isH265NaluHead(dat))
		{
			H265Nalu nalu;
			nalu.pos = pos - 4;
			nalu.naluType = (dat[4] & 0x7e) >> 1;
			vecNaluHead.push_back(nalu);
		}

		bar.update(pos * 1.0 / len);
	}
	in.close();
	bar.finish();

	return vecNaluHead;
}

// ���.len265�ļ�
bool writeH265Len(const std::vector<H265Nalu>& vecNaluHead, const std::string& fileName)
{
	std::uint64_t pre = 0;
	std::uint64_t cur = 0;
	std::ofstream out(fileName, std::ios::out | std::ios::binary);
	if (!out.is_open())
	{
		return false;
	}

	int vps_cnt = 0;
	for (int i = 1; i < vecNaluHead.size(); i++)
	{
		int naluType = vecNaluHead[i].naluType;
		cur = vecNaluHead[i].pos;
		if ((0 <= naluType && naluType <= 21 && naluType != 19 && naluType != 20) || naluType == 32)
		{
			std::int32_t len = static_cast<std::int32_t>(cur - pre);
			out.write((char*)&len, sizeof(std::int32_t));
			pre = cur;
		}

		if (naluType == 32) // VPS����
		{
			vps_cnt++;
		}
	}
	out.close();

	std::cout << "VPS Count: " << vps_cnt << std::endl;

	return true;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "Usage: " << (char*)argv[0] << " parameter1 parameter2" << std::endl;
		std::cout << "Param 1: In.put File Name: *.h265" << std::endl;
		std::cout << "Param 2: Output File Name: *.len265" << std::endl;
		return 0;
	}

	std::string inFileName = (char*)argv[1];   // �����ļ�
	std::string outFileName = (char*)argv[2];  // ����ļ�

	auto found = outFileName.find(".", 0);
	std::string suffix = outFileName.substr(found + 1, outFileName.size() - found - 1);
	if (suffix != "len265")
	{
		std::cout << ">>> output file must be .len265 type" << std::endl;
		return 0;
	}

	auto vecNaluHead = findH265NaluHead(inFileName);

	writeH265Len(vecNaluHead, outFileName);

	return 0;
}