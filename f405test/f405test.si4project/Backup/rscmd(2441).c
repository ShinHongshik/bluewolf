
#include "rscmd.h"


int db_cmd_dummy(void)
{
	return CMD_END;
}

//==========================================================================
#define DB_FUNC(CMD) int CMD ## _db_cmd(void)
//==========================================================================
//int test_db_cmd(void);
DB_FUNC(help);
DB_FUNC(dbg);
DB_FUNC(ver);
//DB_FUNC(ts0);
//DB_FUNC(ts2);
DB_FUNC(ts3);
DB_FUNC(dsw);
DB_FUNC(swh);
DB_FUNC(swa);
DB_FUNC(dcmd);
DB_FUNC(wsw);
DB_FUNC(clm);
DB_FUNC(ivc);
DB_FUNC(ipc);




char * PSTR(flash char * strr)
{
	char buff[64] = {0};
	strcpyf(buff,strr);
	return &buff[0];
}


//==========================================================================
typedef int (*CmdFunction)(void);		// �Լ� ������ ������
typedef struct db_cmd_struct
{
	const char *name;
	CmdFunction cmdfunc;	// �Լ� ������ ����
}cmd_struct;
//==========================================================================
#define DB_COMMAND(CMD)  {#CMD, CMD ## _db_cmd}
//==========================================================================
const cmd_struct db_cmd[] =
{
//	{"test", test_db_cmd},
	DB_COMMAND(help),
	DB_COMMAND(dbg),
	DB_COMMAND(ver),
//	DB_COMMAND(ts0),
//	DB_COMMAND(ts2),
	DB_COMMAND(ts3),
	DB_COMMAND(dsw),
	DB_COMMAND(swh),
	DB_COMMAND(swa),
	DB_COMMAND(dcmd),
	DB_COMMAND(wsw),
	DB_COMMAND(clm),
	DB_COMMAND(ivc),
	DB_COMMAND(ipc)
};

const int db_cmd_list_cnt = sizeof(db_cmd)/sizeof(cmd_struct);
int help_db_cmd(void)
{
	my_put_string (DBG_PORT, PSTR("====== Command List ====== \r\n"));
//	printf_P(PSTR(" test\r\n"));
	my_put_string (DBG_PORT, PSTR("ver\r\n"));
	my_put_string (DBG_PORT, PSTR("dbg\r\n"));
	my_put_string (DBG_PORT, PSTR("ts3\r\n"));
#ifdef USE_BACKUP
	my_put_string (DBG_PORT, PSTR("dsw:disPlayHooker\r\n"));
	my_put_string (DBG_PORT, PSTR("swh:3portSend 0x010203\r\n"));
#endif
	my_put_string (DBG_PORT, PSTR("swa:3portSend 0x010203RN\r\n"));
	my_put_string (DBG_PORT, PSTR("dcmd:wizCmdMode\r\n"));
	my_put_string (DBG_PORT, PSTR("wsw:ipchangeflga => 1\r\n"));
	my_put_string (DBG_PORT, PSTR("clm:1-3port Clone mode\r\n"));
	my_put_string (DBG_PORT, PSTR("ivc:max inverterCnt\r\n"));
	my_put_string (DBG_PORT, PSTR("ipc:wizIP-Change\r\n"));


	my_put_string (DBG_PORT, PSTR("========================== \r\n"));
	return CMD_END;
}

char mycmdcmp_P(const char *cmd, const char *buff)
{
	char cmd_len = 0;

	cmd_len = strlen(cmd);
	if(!strncmp(cmd, buff, cmd_len))
	{
		if(buff[cmd_len]==0 || buff[cmd_len]==' ')
			return 0;
	}

	return 1;
}

#define DB_CMD_BUFLEN		128
char db_cmd_buf[DB_CMD_BUFLEN + 2] = {0};
char db_reaction_buf[128] = {0};
char wiz_cmd_buf[DB_CMD_BUFLEN + 2] = {0};
char db_cmd_buf4toss[32] = {0};

unsigned char  db_cmd_len = 0, wiz_cmd_len = 0;
unsigned char db_cmd_sqc = 0, wiz_cmd_sqc = 0;
unsigned char db_cmd_idx = 0, wiz_cmd_idx = 0;


CmdFunction rs_rece_func = db_cmd_dummy, wiz_rece_func = db_cmd_dummy;

uc8 db_cmd_sub_sqc = 0;
uc08 wiz_cmd_sub_sqc = 0;

unsigned char flagCommSendBusy;
unsigned char flagComm3receiveBusy;


enum PC_REACTION{NO_REACK,CORRECT,TIMEOVER};

//extern void my_put_string_thrDbg(uc08 portNum , char * cbuf);

//extern void wiz_driver(void);

void rs_rece_db(void)
{
	char data , dtBuf ;
	static char olddata;
	char dcmdbuf[128] = {0};
	static uc08 check_toss= 0;
//	static char cmdEndstandbyFlag = 0;

	switch(db_cmd_sqc)
	{
	case 0:
		db_cmd_len = 0;
		db_cmd_idx = 0;
		db_cmd_sub_sqc = 0;
		db_cmd_sqc++;
		break;
	case 1:
		if(rx_wr_index1 != rx_rd_index1)
		{
			data = getchar1 ();
			switch(data)
			{
				case '!':
					check_toss = 1;
				break;
				case '#':
					check_toss = 3;
				break;
				case '$':
					check_toss = 4;
				break;
				case 0x0D:	//CR
					if(check_toss != 0) goto CONU;
					db_cmd_buf[db_cmd_len]= 0;
				break;
				case 0x0A:
					//auto_read_cnt = 7;
					if(check_toss != 0)
					{
						db_cmd_buf[db_cmd_len++]= data;
						db_cmd_sqc= 4;
						break;
					}
					db_cmd_buf[db_cmd_len]= 0;
					if((db_cmd_len > 0)&&(db_cmd_len < DB_CMD_BUFLEN))
					{
						db_cmd_sqc++;
						break;
					}
					db_cmd_len = 0;
				break;
				default:
//					cmdEndstandbyFlag = 0;
				CONU:

					if(db_cmd_len >= DB_CMD_BUFLEN)	db_cmd_len = DB_CMD_BUFLEN;
					db_cmd_buf[db_cmd_len] = data;
					olddata = data;
					db_cmd_len++;
					break;
			}
		}
		break;
	case 2:
	findloop:
		if(!mycmdcmp_P(db_cmd[db_cmd_idx].name, db_cmd_buf))
		{
			rs_rece_func = db_cmd[db_cmd_idx].cmdfunc;
			db_cmd_sqc++;
			break;
		}
		else
		{
			db_cmd_idx++;
			if(db_cmd_idx >= db_cmd_list_cnt)
			{
				if(dbgLevel >= 0)
				{
					sprintf(dcmdbuf,"?1<");
					memcpy(dcmdbuf + 3,db_cmd_buf, db_cmd_len);
					my_Nput_string (DBG_PORT, dcmdbuf, (db_cmd_len + 3) );
					//sbi (flagSendData , DBG_PORT);
				}
				db_cmd_sqc = 0;
			}
			else
			{
				goto findloop;	//�Լ� �������� �ʰ� ��� ã��
			}
		}
		break;
	case 3:
		if(rs_rece_func() == CMD_END)
		{
			db_cmd_sub_sqc = 0;
			db_cmd_sqc = 0;
		}
		break;
	case 4:   // toss mode
		if(dbgLevel >= 0)
		{
			switch (check_toss)
			{
				case 1:
					strncpy(dcmdbuf ,db_cmd_buf, db_cmd_len);
					my_put_string (RS485_PORT, dcmdbuf);
				break;
				case 3:
					strncpy(dcmdbuf ,db_cmd_buf, db_cmd_len);
					my_put_string (WCDMA_PORT, dcmdbuf);
				break;
				case 4:
					strncpy(dcmdbuf ,db_cmd_buf, db_cmd_len);
					my_put_string (W107_PORT, dcmdbuf );
				break;
				default:
				break;
			}
			check_toss = 0 ;
			//sbi (flagSendData , DBG_PORT);
		}

		db_cmd_sub_sqc = 0;
		db_cmd_sqc = 0;
	default:
		db_cmd_sqc = 0;
		break;
	}
}


#if 0
char cmdPasingFromW107(int timeover)
{
	static char cfSqc = 0;
	static int timer = 0;
//	char * sendCmd = tx_hookingbuf;
	char * receiveCmd = wiz_cmd_buf;
	char ret ;

	switch(cfSqc)
	{
		case 0:
			timer = clk_get_curr_ms ();
			cfSqc++;
		break;
		case 1:
//			if(strncmp(sendCmd , receiveCmd, 2) == 0) return CORRECT;
			if((sysCnt - timer) < timeover) break;
			cfSqc = 0;
			return TIMEOVER;
		default:
			cfSqc = 0;
		break;

	}
	return NO_REACK;
}
#endif

void rs_rece_w107(void)
	{
		char data , dtBuf  ;
		static char predata ;
		char cmdbuf[128] = {0} ;

		switch(wiz_cmd_sqc)
		{
		case 0:
			wiz_cmd_len = 0;
			wiz_cmd_idx = 0;
			wiz_cmd_sub_sqc = 0;
			wiz_cmd_sqc++;
			break;
		case 1:
			if(rx_wr_index3 != rx_rd_index3)
			{
				flagCommSendBusy = USARTREACTIONDELAY;
				data = getchar3 ();

				switch(data)
				{
				case 0x0A:	//CR
					if(predata != 0x0D) goto CONT_W;
					//auto_read_cnt = 7;

					wiz_cmd_buf[wiz_cmd_len++] = data;
					if((wiz_cmd_len > 0)&&(wiz_cmd_len < DB_CMD_BUFLEN))
					{
						wiz_cmd_buf[wiz_cmd_len] = 0;
						wiz_cmd_sqc++;
						break;
					}
					wiz_cmd_len = 0;
					break;
				default:
	//				if(dbgLevel > 0)
	//					putchar1 (data);
				CONT_W:
					if(wiz_cmd_len >= DB_CMD_BUFLEN)
						wiz_cmd_len = DB_CMD_BUFLEN;
					wiz_cmd_buf[wiz_cmd_len] = data;
					wiz_cmd_len++;
					predata = data;
					break;
				}
			}
			if((invSwitchFlag == 0) && (flagCommSendBusy == 0))
			{
				if(wiz_cmd_len > 0)
				wiz_cmd_sqc = 3;
			}
			break;
		case 2:
				flagUart3ReactionPasingComp = wiz_pasing_data(wiz_cmd_buf);
				if((dbgLevel >= 0) && (swUsart4dbgshow & bv(USARTDBGSWHOW_IN3)))
				{
					sprintf(cmdbuf,"3<%s:C%0d",wiz_cmd_buf,flagUart3ReactionPasingComp);
					my_Nput_string (DBG_PORT, cmdbuf, wiz_cmd_len + 2 );

					//my_Nput_string (DBG_PORT, wiz_cmd_buf, wiz_cmd_len );
					//sbi (flagSendData , DBG_PORT);

				}
			wiz_cmd_sub_sqc = 0;
			wiz_cmd_sqc = 0;
			break;
		case 3:
			if(swUsart4dbgshow & bv(USARTDBGSWHOW_IN3))
			{
				sprintf(cmdbuf,"3<");
				strncpy(cmdbuf+2,wiz_cmd_buf,wiz_cmd_len);
				my_Nput_string (DBG_PORT, cmdbuf, wiz_cmd_len + 2 );
			}
			if(cmdPasingFromW107() == CMD_CONT ) wiz_cmd_sqc = 1;
			else wiz_cmd_sqc = 0;
			break;
		default:
			wiz_cmd_sqc = 0;
			break;
		}
	}


int dsw_db_cmd(void)
{
	int c, val;
	char buff[32] ={0};

	switch(db_cmd_sub_sqc)
	{
	case 0:
		if(!db_cmd_buf[3])
			goto end;
		db_cmd_sub_sqc++;
		break;
	case 1:
		c = sscanf(db_cmd_buf + 3, "%d", &val);
		if(c > 0)
		{
			if(val>=1 && val<5)
			{
				val -= 	1;
				if(swUsart4dbgshow & bv(val))
				{
					cbi (swUsart4dbgshow  , val);
				}
				else
				{
					//swUsart4dbgshow &=  0xF0;
					sbi(swUsart4dbgshow  , val);
				}
			}
			else if(val>=5 && val<9)
			{
				val -= 	1;
				if(swUsart4dbgshow & bv(val))
				{
					cbi (swUsart4dbgshow  , val);
				}
				else
				{
					//swUsart4dbgshow &=  0x0F;
					sbi(swUsart4dbgshow  , val);
				}
			}
		}
	default:
	end:
		if(dbgLevel > 0)
		{
			sprintf(db_reaction_buf,"swUsart4dbgshow=0x%x\r\n", swUsart4dbgshow);
			my_put_string (DBG_PORT, db_reaction_buf);
		}
		return CMD_END;
	}
	return CMD_CONT;
}

int ivc_db_cmd(void)
{
	int c, val;
	char buff[32] ={0};

	switch(db_cmd_sub_sqc)
	{
	case 0:
		if(!db_cmd_buf[3])
			goto end;
		db_cmd_sub_sqc++;
		break;
	case 1:
		c = sscanf(db_cmd_buf + 3, "%d", &val);
		if(c > 0)
		{
			if(val>=0 && val<21)
				eeInverterCount = val;
		}
		db_cmd_sub_sqc++;
		break;
	default:
	end:
			sprintf(buff,"eeInvCnt=,%d\r\n", eeInverterCount);
			my_put_string (DBG_PORT, buff);
		return CMD_END;
	}
	return CMD_CONT;
}


int dbg_db_cmd(void)
{
	int c, val;
	char buff[32] ={0};

	switch(db_cmd_sub_sqc)
	{
	case 0:
		if(!db_cmd_buf[3])
			goto end;
		db_cmd_sub_sqc++;
		break;
	case 1:
		c = sscanf(db_cmd_buf + 3, "%d", &val);
		if(c > 0)
		{
			if(val>=0 && val<10)
				dbgLevel = val;
		}
		db_cmd_sub_sqc++;
		break;
	default:
	end:
			sprintf(buff,"gbgLevel=,%d\r\n", dbgLevel);
			my_put_string (DBG_PORT, buff);
		return CMD_END;
	}
	return CMD_CONT;
}

int ts3_db_cmd(void)
	{
		int c, val;
		char buff[128] ={0};
		static int num = 0;

		switch(db_cmd_sub_sqc)
		{
		case 0:
			if(!db_cmd_buf[3])
				goto end;
			db_cmd_sub_sqc++;
			break;
		case 1:
			sprintf(buff,"%s\r\n",&db_cmd_buf[4]);
			my_put_string (W107_PORT, buff);
		default:
		end:
			return CMD_END;
		}
		return CMD_CONT;

	}

int swh_db_cmd(void)
	{
		int c, val;
		char buff[32] ={0};

//		sprintf(buff,"%c%c%c", 0x1,0x2,0x3);
//		my_Nput_string (W107_PORT, buff, 3);
		putchar3 (0x02);
		putchar3 (0x03);
		UDR3 = 0x01;

		return CMD_END;
	}

int swa_db_cmd(void)
{
	int c, val;
	char buff[32] ={0};

	sprintf(buff,"%c%c%c\r\n", 0x1,0x2,0x3);
	my_Nput_string (W107_PORT, buff, 5);

	return CMD_END;

}

int clm_db_cmd(void)
{

	if(swUsart3clone == 1)
	swUsart3clone = 0;
	else swUsart3clone = 1;

	if(dbgLevel > 0)
	{
		sprintf(db_reaction_buf,"Usart3clone=%s\r\n", swUsart3clone );
		my_put_string (DBG_PORT, db_reaction_buf);
	}
	return CMD_END;
}


int ver_db_cmd(void)
{
	sprintf(db_reaction_buf,"H/W : %f\r\nF/W : %f\r\n", (float)0.1 ,(float)0.1   );
	return CMD_END;
}

int wsw_db_cmd(void)
{
	invSwitchFlag = 1;
	return CMD_END;
}

int ipc_db_cmd(void)
{
	if(invChageSw == 1)
	invChageSw = 0;
	else invChageSw = 1;
	return CMD_END;
}
int dcmd_db_cmd(void)
{
	if(wizCmdmodeOn == 1)
	wizCmdmodeOn = 0;
	else wizCmdmodeOn = 1;

	if(dbgLevel > 0)
	{
		sprintf(db_reaction_buf,"wizCmdmodeOn=%d\r\n", wizCmdmodeOn);
		my_put_string (DBG_PORT, db_reaction_buf);
	}
	return CMD_END;
}


char my_Nput_string(uc08 portNum, char *addr_oribuf,int length)
	{
		int send_size ;
		char first ;
		char * addr = addr_oribuf;
		volatile uc08 saveVal;


		first = *addr++;
		if(portNum > 3) return 1;
		if(length == 0) return 1;


		send_size = length;
		if(send_size >= 0xff) send_size = 0xff ;


		switch(portNum)
		{
			case RS485_PORT:
				while(--send_size)
				{
					putchar0(*addr);
					*addr =0;
					addr++;
				}
				direct0:
				TXEN=1;TXEN=1;
				UDR0 = first;
			break;
			case DBG_PORT:
				while(--send_size)
				{
					putchar1(*addr);
					*addr =0;
					addr++;
				}
				direct1:
				UDR1 =  first;
			break;
			case WCDMA_PORT:
				while(--send_size)
				{
					putchar2(*addr);
					*addr =0;
					addr++;
				}
				direct2:
				UDR2 = first;
			break;
			case W107_PORT:
				while(--send_size)
				{
					putchar3(*addr);
					*addr =0;
					addr++;
				}
				direct3:
				UDR3 = first;
			break;
		}
	return 0;
	}



char my_put_string(uc08 port,char *maddr)
	{
		uc08 send_size = 0;
		char first ;
		int i ,  c ;
		char* addr = maddr;
		volatile uc08 oldclonemode = 0;

		first = *addr++;
		if(first == 0) return 1;
		if(port > 3) return 1;

		switch(port)
		{
			case RS485_PORT:
				if(*addr == 0) break;
				while(*addr)
				{
					putchar0(*addr);
					addr++;
					send_size++;
					if(send_size > 254)break;

				}
				write_dudr (RS485_PORT, first);
			break;
			case DBG_PORT:
				if(*addr == 0) break;
				while(*addr)
				{
					putchar1(*addr);
					addr++;
					send_size++;
					if(send_size > 254)break;
				}
				write_dudr (DBG_PORT, first);
			break;
			case WCDMA_PORT:
				if(*addr == 0) break;
				while(*addr)
				{
					putchar2(*addr);
					addr++;
					send_size++;
					if(send_size > 254)break;
				}
				write_dudr (WCDMA_PORT, first);
			break;
			case W107_PORT:
				if(*addr == 0) break;
				while(*addr)
				{
					putchar3(*addr);
					addr++;
					send_size++;
					if(send_size > 254)break;
				}
				write_dudr (W107_PORT, first);
			break;
			default:
			break;
		}
	return 0;
	}


