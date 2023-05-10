#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include "CustomTradeSpi.h"

#include"httplib.h"
//#include <windows.h> 

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"



using namespace std;

// ���ӿ�
#pragma comment (lib, "thostmduserapi_se.lib")
#pragma comment (lib, "thosttraderapi_se.lib")

// ---- ȫ�ֱ��� ---- //
// ��������
TThostFtdcBrokerIDType gBrokerID = "9999";                         // ģ�⾭���̴���
TThostFtdcInvestorIDType gInvesterID;                         // Ͷ�����˻���
TThostFtdcPasswordType gInvesterPassword;                     // Ͷ��������


// ���ײ���
CThostFtdcTraderApi* g_pTradeUserApi = nullptr;                    // ����ָ��
//char gTradeFrontAddr[] = "tcp://180.168.146.187:10130";            // ģ�⽻��ǰ�õ�ַ,����ʱ�����Ҫ����
//char gTradeFrontAddr[] = "tcp://180.168.146.187:10201";            // ģ�⽻��ǰ�õ�ַ,����ʱ�����Ҫ����

char gTradeFrontAddr[]= "tcp://180.168.146.187:10130";

TThostFtdcInstrumentIDType g_pTradeInstrumentID = "IF2304";        // �����׵ĺ�Լ����
TThostFtdcDirectionType gTradeDirection = THOST_FTDC_D_Sell;       // ��������
TThostFtdcPriceType gLimitPrice = 22735;                           // ���׼۸�


vector <string>vecInvesterID = {};  // Ͷ�����˻���
vector <string>vecInvesterPassword = {};  // Ͷ��������
vector <string>vecTradeFrontAddr = {};//Ͷ����ǰ�û�
vector <int>vecContractAlertNum = {};//Ͷ����ContractAlertNum
vector <int>vecAllContractAlertNum = {};//Ͷ����AllContractAlertNum


string readFromJsonData(string msg) // msgΪ�����json��ʽ�ļ�{"cmd":"getType","whichType":"type1","result":0}
{
	rapidjson::Document doc;
	doc.Parse(msg.c_str());
	if (!doc.IsObject())  // �ж��ǲ���json��ʽ���ļ�
	{
		return "is not a jsonfile";
	}
	if (!doc.HasMember("Alluser"))  // �жϼ����Ƿ����"cmd"
	{
		//std::cout<<"is not contain cmd"<<std::endl;
		return "is not contain cmd";
	}


	rapidjson::Value& m = doc["Alluser"];
	if (m.IsArray()) {
		for (int i = 0; i < m.Size(); i++) {
			rapidjson::Value& id = m[i]["InvesterID"];
			rapidjson::Value& pw = m[i]["InvesterPassword"];
			rapidjson::Value& tf = m[i]["TradeFrontAddr"];
			rapidjson::Value& cn = m[i]["ContractAlertNum"];
			rapidjson::Value& an = m[i]["AllContractAlertNum"];
			vecInvesterID.push_back(id.GetString());
			vecInvesterPassword.push_back(pw.GetString());
			vecTradeFrontAddr.push_back(tf.GetString());
			
			vecContractAlertNum.push_back(atoi(cn.GetString()));
			vecAllContractAlertNum.push_back(atoi(an.GetString()));
		}
	}

	rapidjson::Value& jsonValue = doc["Alluser"][0]["InvesterID"]; // ��ȡֵ

	if (!jsonValue.IsString())  // �жϽ���ǲ����ַ��� ��toInt()�ж��ǲ������� ����������
	{
		return "is not string type";
	}
	return  jsonValue.GetString();
}

string readfile(const char* filename) {
	//FILE* fp = fopen(filename, "rb");

	FILE* fp;
	errno_t err = fopen_s(&fp, filename, "rb");

	if (!fp) {
		printf("open failed! file: %s", filename);
		return "";
	}


	char buf[1024 * 16]; //�½�������
	string result;
	/*ѭ����ȡ�ļ���ֱ���ļ���ȡ���*/
	while (int n = fgets(buf, 1024 * 16, fp) != NULL)
	{
		result.append(buf);
		//cout << buf << endl;
	}
	fclose(fp);
	return result;
}

//ȡ����ײ�������
//string getHead(const string& str)
//{
//	
//	char c;
//	c = str[1];//��ȡ�ڶ����ַ���
//	if (c >= '0' && c <= '9')
//	{	
//		//printf("������\n");
//		return str.substr(0, 1);
//	}
//	else  if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
//	{
//		//printf("����ĸ\n");
//		return str.substr(0, 2);
//	}
//	
//	else
//	{
//		printf("�������ַ�\n");
//	}
//	
//
//}



int main()
{	


	std::string jsonstr = readfile(".\\userlist.json");//���ļ�ת��Ϊjsonstr�ַ���
	std::string msg = readFromJsonData(jsonstr);//��jsonstrת����vecInvesterID��vecInvesterPassword
	int user_Length = vecInvesterID.size();

	vector<CThostFtdcTraderApi*> m_vecpTradeUserApi;
	vector<CustomTradeSpi*> m_vecTradeSpi;

	for (int i = 0; i < user_Length; i++) {
		strcpy(gInvesterID, vecInvesterID[i].c_str());
		strcpy(gInvesterPassword, vecInvesterPassword[i].c_str());
		strcpy(gTradeFrontAddr, vecTradeFrontAddr[i].c_str());

		//��ʼ�������߳�
		std::cout << "��ʼ������..." << endl;



		g_pTradeUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi(); // ��������ʵ��
		CustomTradeSpi* pTradeSpi = new CustomTradeSpi(vecInvesterID[i],vecContractAlertNum[i],vecAllContractAlertNum[i]);               // �������׻ص�ʵ��

		
		g_pTradeUserApi->RegisterSpi(pTradeSpi);                      // ע���¼���
		g_pTradeUserApi->SubscribePublicTopic(THOST_TERT_RESTART);    // ���Ĺ�����
		g_pTradeUserApi->SubscribePrivateTopic(THOST_TERT_RESTART);   // ����˽����
		g_pTradeUserApi->RegisterFront(gTradeFrontAddr);              // ���ý���ǰ�õ�ַ
		g_pTradeUserApi->Init();                                      // ��������

		m_vecpTradeUserApi.push_back(g_pTradeUserApi);
		m_vecTradeSpi.push_back(pTradeSpi);

		//������������ִ����ϣ�������һ��ѭ��������һֱ����
		while (!pTradeSpi->taskdone) {
			Sleep(10);
		}
	}



	
	
		//for (auto& item : m_vecTradeSpi[0]->m_ContractInstmap)
		//{
		//	cout <<"m_ContractInstmap key:"<<item.first << endl;
		//	for (auto& si : item.second)
		//	{
		//		cout << "m_ContractInstmap value instname: "<<si.first << endl;
		//		cout << "m_ContractInstmap value instcount: "<<si.second << endl;
		//	}
		//}

		//for (auto& item : m_vecTradeSpi[0]->m_ContractNummap)
		//{
		//	cout << "m_ContractNummap key:" << item.first << endl;
		//	cout << "m_ContractNummap value count: " << item.second << endl;
		//
		//}

		//cout << m_vecTradeSpi[0]->totalContractNum << endl;
	std::cout << "ALL task done" << endl;
	int i=getchar();

	return 0;
}

