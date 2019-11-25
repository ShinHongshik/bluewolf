
#include "global_hs.h"

#define DBG_PORT 1

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
//DB_FUNC(ts3);
//DB_FUNC(dsw);


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
	DB_COMMAND(ver)
//	DB_COMMAND(ts0),
//	DB_COMMAND(ts2),
};

const int db_cmd_list_cnt = sizeof(db_cmd)/sizeof(cmd_struct);
int help_db_cmd(void)
{
	my_put_string ( "====== Command List ====== \r\n");
//	printf_P(PSTR(" test\r\n"));
	my_put_string ( "ver\r\n");
	my_put_string ( "dbg\r\n");
	my_put_string ( "ts3\r\n");
#ifdef USE_BACKUP
	my_put_string ("dsw:disPlayHooker\r\n");
	my_put_string ("swh:3portSend 0x010203\r\n");
#endif
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

uc08 db_cmd_sub_sqc = 0;
uc08 wiz_cmd_sub_sqc = 0;

unsigned char flagCommSendBusy;
unsigned char flagComm3receiveBusy;


enum PC_REACTION{NO_REACK,CORRECT,TIMEOVER};

//extern void my_put_string_thrDbg(uc08 portNum , char * cbuf);

//extern void wiz_driver(void);

void rs_rece_db(void)
{
	char dtBuf ;
  uc08 data;
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
		while((HAL_UART_Receive (&huart2, &data, 1, 10)) != HAL_ERROR)
		{
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
			strncpy(dcmdbuf ,db_cmd_buf, db_cmd_len);
			my_put_string ( dcmdbuf);
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
			my_put_string (buff);
		return CMD_END;
	}
	return CMD_CONT;
}


int ver_db_cmd(void)
{
	sprintf(db_reaction_buf,"H/W : %f\r\nF/W : %f\r\n", (float)0.1 ,(float)0.1   );
	return CMD_END;
}


