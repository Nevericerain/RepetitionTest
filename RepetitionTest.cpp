//============================================================================================
//    There are many factors which may make a difference to the probability of the repetition.
//    We mainly consider six of them in our experimentation:
//    (1) The PF upper size
//    (2) The pace of changing a PF¡¯s size
//    (3) The count of PFs
//    (4) The sequence of moving PFs
//    (5) The size of free space in the stable storage device
//    (6) The measurement of PF location
//    You can study these factors by changing this source code.
//    In this test program, we mainly study factor(6).We measure a PF's location by extracting
//its first and last sector cluster numbers in the hard disk. All the sizes of PFs ate controlled
//within 512KB. The count of PFs is three. We move PFs in an ascending number sequence. As to 
//factor(2), the size change cycle of PF1, PF2 and PF3 are 83, 97 and 113, and we compute their
//size change paces according to the size change cycles.
//    83*97*113 = 909763
//    Therefore, The PFs can concurrently return to zero after 909763 moves
//============================================================================================
#include <windows.h> 
#include <assert.h> 
#include <iostream>
#include <io.h>   

#include <string>
#include <fstream>

using namespace std;

BOOL GetFileClusters(LPCTSTR lpszFileName) 
{ 
    DWORD dwExtentIndex = 0; 
    assert(lpszFileName != NULL); 
    if (lpszFileName == NULL) 
        return FALSE; 
    HANDLE hFile = CreateFile(lpszFileName, FILE_READ_DATA || FILE_WRITE_DATA || FILE_APPEND_DATA || FILE_EXECUTE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,NULL); 
    if (hFile == INVALID_HANDLE_VALUE) 
        return FALSE; 
    STARTING_VCN_INPUT_BUFFER StartVcnInbuff = {0}; 
    BOOL bIsOK = TRUE; 
    DWORD dwInfoSize = sizeof (RETRIEVAL_POINTERS_BUFFER); 
    DWORD dwReciveLen = 0; 
    LARGE_INTEGER ExtentLength; 
    LARGE_INTEGER TotalClusters; 
    ExtentLength.QuadPart = 0; 
    TotalClusters.QuadPart = 0; 
    while (TRUE) 
    { 
        PRETRIEVAL_POINTERS_BUFFER pRetrievalBuff = (PRETRIEVAL_POINTERS_BUFFER)GlobalAlloc(GPTR, dwInfoSize); 
        if (pRetrievalBuff == NULL) 
        { 
            bIsOK = FALSE; 
            break; 
        } 
        bIsOK = DeviceIoControl(hFile, FSCTL_GET_RETRIEVAL_POINTERS, &StartVcnInbuff, sizeof(STARTING_VCN_INPUT_BUFFER),pRetrievalBuff, dwInfoSize, &dwReciveLen, NULL); 
        if (bIsOK) 
        { 
            for (UINT32 i = 0; i < pRetrievalBuff->ExtentCount; i++) 
            { 
                ExtentLength.QuadPart = pRetrievalBuff->Extents[i].NextVcn.QuadPart - pRetrievalBuff->StartingVcn.QuadPart; 
                //printf("Extent[%d] LCN = %I64ld \t VCN = %I64ld \t Clusters = %I64ld/End LCN = %I64ld\n", dwExtentIndex, pRetrievalBuff->Extents[i].Lcn,pRetrievalBuff->StartingVcn.QuadPart, ExtentLength, pRetrievalBuff->Extents[i].Lcn.QuadPart + ExtentLength.QuadPart - 1); 
                TotalClusters.QuadPart += ExtentLength.QuadPart; 
                StartVcnInbuff.StartingVcn = pRetrievalBuff->Extents[i].NextVcn; 
                dwExtentIndex ++; 
            } 

            FILE *fp;  
            fp=fopen("E:\\test\\result.txt","a"); 
            if (fp!=NULL)
			{
				fprintf(fp,"%I64ld \t", pRetrievalBuff->Extents[0].Lcn);
				fprintf(fp,"%I64ld \t", pRetrievalBuff->Extents[pRetrievalBuff->ExtentCount-1].Lcn.QuadPart + ExtentLength.QuadPart - 1);
			}
            fclose(fp);
            break; 
        } 
        else 
        { 
            DWORD dwErrorCode = GetLastError(); 
            if (dwErrorCode == ERROR_INSUFFICIENT_BUFFER) 
            { 
                GlobalFree(pRetrievalBuff); 
                pRetrievalBuff = NULL; 
                dwInfoSize *= 2; 
                continue; 
            } 
            else if (dwErrorCode == ERROR_MORE_DATA) 
            { 
                for (UINT32 i = 0; i < pRetrievalBuff->ExtentCount; i++) 
                { 
                    ExtentLength.QuadPart = pRetrievalBuff->Extents[i].NextVcn.QuadPart - pRetrievalBuff->StartingVcn.QuadPart; 
                    //printf("Extent[%d] LCN = %I64ld \t VCN = %I64ld \t Clusters = %I64ld/End LCN = %I64ld\n", dwExtentIndex, pRetrievalBuff->Extents[i].Lcn,pRetrievalBuff->StartingVcn.QuadPart, ExtentLength, pRetrievalBuff->Extents[i].Lcn.QuadPart + ExtentLength.QuadPart - 1); 
                    TotalClusters.QuadPart += ExtentLength.QuadPart; 
                    StartVcnInbuff.StartingVcn = pRetrievalBuff->Extents[i].NextVcn; 
                    dwExtentIndex++; 
                } 
                continue; 
            } 
            else 
            { 
                GlobalFree(pRetrievalBuff); 
                pRetrievalBuff = NULL; 
                //printf("Error:%d\n", dwErrorCode); 
            } 
        } 
    } 
    CloseHandle(hFile); 
    return bIsOK; 
} 

static string Pace_128Byte = "11111111110000000000111111111100000000001111111111000000000011111111110000000000111111111100000000001111111111000000000088888888";


void ChangPFSize_Pace1(string str)        
{
	if(_access(str.c_str(), 0))
	{
		cout<<"file not exist!"<<endl;
		return;
	}
	else
	{
		ofstream ofs;
		ofs.open(str,ios_base::app);
		int count = 0;
		while (count < 49)                //512KB/83/128B = 49
		{
			ofs<<Pace_128Byte;
			count++;
		}
		ofs<<"\n";
		ofs.close();
	}
}

void ChangPFSize_Pace2(string str)
{
	if(_access(str.c_str(), 0))
	{
		cout<<"file not exist!"<<endl;
		return;
	}
	else
	{
		ofstream ofs;
		ofs.open(str,ios_base::app);
		int count = 0;
		while (count < 42)               //512KB/97/128B = 42
		{
			ofs<<Pace_128Byte;
			count++;
		}
		ofs<<"\n";
		ofs.close();
	}
}

void ChangPFSize_Pace3(string str)
{
	if(_access(str.c_str(), 0))
	{
		cout<<"file not exist!"<<endl;
		return;
	}
	else
	{
		ofstream ofs;
		ofs.open(str,ios_base::app);
		int count = 0;
		while (count < 36)               //512KB/113/128B = 36
		{
			ofs<<Pace_128Byte;
			count++;
		}
		ofs<<"\n";
		ofs.close();
	}
}

void MovePF1()
{
	ChangPFSize_Pace1("E:\\test\\PF1.txt");
	system("copy E:\\test\\PF1.txt   E:\\test\\tempPF1.txt >nul");
	system("del  E:\\test\\PF1.txt >nul");
	system("ren  E:\\test\\tempPF1.txt   PF1.txt >nul");
}

void MovePF2()
{
	ChangPFSize_Pace2("E:\\test\\PF2.txt");
	system("copy E:\\test\\PF2.txt   E:\\test\\tempPF2.txt >nul");
	system("del  E:\\test\\PF2.txt >nul");
	system("ren  E:\\test\\tempPF2.txt   PF2.txt >nul");
}

void MovePF3()
{
	ChangPFSize_Pace3("E:\\test\\PF3.txt");
	system("copy E:\\test\\PF3.txt   E:\\test\\tempPF3.txt >nul");
	system("del  E:\\test\\PF3.txt >nul");
	system("ren  E:\\test\\tempPF3.txt   PF3.txt >nul");
}


int main(int argc, CHAR* argv[]) 
{ 
	FILE *fp;  
    fp=fopen("E:\\test\\result.txt","a"); 
    if (fp!=NULL)
	{
		fprintf(fp,"\nNumber\tPF1start\tPF1end  \tPF2start\tPF2end  \tPF3start\tPF3end\n");
	}
    fclose(fp);
	int number = 0;
	while(number < 50000)
	{
		if(number % 83 == 0)
		{
			FILE * fp;
			char filename[]="E:\\test\\PF1.txt";
			fp=fopen(filename,"w");                 //Empty PF1
			fclose(fp);
		}
		if(number % 97 == 0)
		{
			FILE * fp;
			char filename[]="E:\\test\\PF2.txt";
			fp=fopen(filename,"w");                 //Empty PF2
			fclose(fp);
		}
		if(number % 113 == 0)
		{
			FILE * fp;
			char filename[]="E:\\test\\PF3.txt";
			fp=fopen(filename,"w");                 //Empty PF3
			fclose(fp);
		}
		MovePF1();
		MovePF2();
		MovePF3();
		ofstream ofs;
        ofs.open("E:\\test\\result.txt",ios_base::app);
        ofs<<'\n'<<number<<"\t";
        ofs.close();
		GetFileClusters(TEXT("E:\\test\\PF1.txt"));
		GetFileClusters(TEXT("E:\\test\\PF2.txt"));
		GetFileClusters(TEXT("E:\\test\\PF3.txt"));
		number++;	
	}
    return 0; 
}
