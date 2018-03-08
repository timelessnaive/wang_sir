#include "MyDirectX.h"

time_t st=0, en1=0,en2=0,en3=0,en4=0;//计时

float sp_bucket_x=0.72, sp_bomb1 = 0.55, sp_bomb2 = 0.45,sp_ex=0.003;//炸弹，篮子速度

time_t st_kiana=0, time_kiana=0;
int f_kiana = 0;//kiana时间

int bomb_count=0;//炸弹计数器
int space_count = 0;

CSound *sound_bomb = NULL;
CSound *sound_kiana = NULL;
CSound *sound_bgm = NULL;

const string APPTITLE = "炸弹游戏";
const int SCREENW = 900;
const int SCREENH = 768;

LPDIRECT3DTEXTURE9 bomb1_surf = NULL;
LPDIRECT3DTEXTURE9 bomb2_surf = NULL;
LPDIRECT3DTEXTURE9 bomb_ex_surf = NULL;

LPDIRECT3DTEXTURE9 num[10] = { NULL };//记分板

LPDIRECT3DTEXTURE9 bucket_surf = NULL;

struct BOMB
{
    float x,y;
	bool sq=0;//是否有特殊效果
    void reset()
    {
        x = (float)(rand() % (SCREENW-128));
        y = 0;
		if (rand() % 100 <25) sq = 1;
		else sq = 0;
    }
};

BOMB bomb1;//炸弹1
BOMB bomb2;//炸弹2，可能有特殊效果

struct BUCKET
{
    float x,y;
};

BUCKET bucket;

int score = 0;
int vibrating = 0;

LPDIRECT3DSURFACE9 surface = NULL;
bool Game_Init(HWND window)
{
    Direct3D_Init(window, SCREENW, SCREENH, false);
    DirectInput_Init(window);
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	HRESULT result = d3ddev->CreateOffscreenPlainSurface(
		SCREENW,
		SCREENH,
		D3DFMT_X8R8G8B8,
		D3DPOOL_DEFAULT,
		&surface,
		NULL);

	if (result != D3D_OK) return false;

	result = D3DXLoadSurfaceFromFile(
		surface,
		NULL,
		NULL,
		"background.jpg",//背景文件名
		NULL,
		D3DX_DEFAULT,
		0,
		NULL);

	srand((unsigned int)time(NULL));
	bomb1.reset();
	bomb2.reset();

    bomb1_surf = LoadTexture("bomb1.bmp", D3DCOLOR_XRGB(255, 255, 255));
	bomb2_surf = LoadTexture("bomb2.bmp", D3DCOLOR_XRGB(255, 255, 255));
	bomb_ex_surf = LoadTexture("bomb_ex.bmp", D3DCOLOR_XRGB(255, 255, 255));
	bucket_surf = LoadTexture("bucket.jpg", D3DCOLOR_XRGB(255, 255, 255));//

	num[0] = LoadTexture("0.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[1] = LoadTexture("1.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[2] = LoadTexture("2.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[3] = LoadTexture("3.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[4] = LoadTexture("4.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[5] = LoadTexture("5.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[6] = LoadTexture("6.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[7] = LoadTexture("7.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[8] = LoadTexture("8.jpg", D3DCOLOR_XRGB(255, 255, 255));
	num[9] = LoadTexture("9.jpg", D3DCOLOR_XRGB(255, 255, 255));

    if (!bomb1_surf) {
        MessageBox(window, "Error loading bomb1","Error",0);
        return false;
    }

	if (!bomb2_surf) {
		MessageBox(window, "Error loading bomb2", "Error", 0);
		return false;
	}

    if (!bucket_surf)
	{
        MessageBox(window, "Error loading bucket","Error",0);
        return false;
    }

	if (!DirectSound_Init(window))
	{
		MessageBox(window, "Error initializing DirectSound", APPTITLE.c_str(), 0);
		return false;
	}
	sound_bomb = LoadSound("step.wav");
	if (!sound_bomb)
	{
		MessageBox(window, "Error loading step.wav", APPTITLE.c_str(), 0);
		return false;
	}

	if(rand()%3==0) sound_bgm = LoadSound("bgm.wav");
	else if(rand() % 3 == 1) sound_bgm = LoadSound("bgm1.wav");
	else if (rand() % 3 == 2) sound_bgm = LoadSound("bgm2.wav");

	sound_kiana = LoadSound("kiana.wav");

    //position the bucket
    bucket.x = 500;
    bucket.y = 630;

	st = time(NULL);//记录开始时间
    return true;
}

void Game_Run(HWND window)
{
    if (!d3ddev) return;

    DirectInput_Update();

	PlaySound(sound_bgm);
    
    bomb1.y += sp_bomb1;//炸弹1速度会逐渐加快
	bomb2.y += sp_bomb2;//炸弹2速度会逐渐加快

    //游戏结束判定
    if (bomb1.y > SCREENH||bomb2.y>SCREENH) 
    {
        MessageBox(0,"YOU DIED！","GAME OVER",0);
        gameover = true;
    }

	//鼠标操作
    /*int mx = Mouse_X();
    if (mx < 0) bucket.x -= sp_bucket_x;
    else if (mx > =0) bucket.x += sp_bucket_x;*/
    
    //键盘操作
    if (Key_Down(DIK_LEFT)) bucket.x -= sp_bucket_x;
    else if (Key_Down(DIK_RIGHT)) bucket.x += sp_bucket_x;
	if (f_kiana)
	{
		if (Key_Down(DIK_UP)) bucket.y -= sp_bucket_x;
		else if (Key_Down(DIK_DOWN)) bucket.y += sp_bucket_x;
	}

    //手柄操作
    if (XInput_Controller_Found())
    {
        //左摇杆
        if (controllers[0].sThumbLX < -5000) 
            bucket.x -= sp_bucket_x;
        else if (controllers[0].sThumbLX >= 5000) 
            bucket.x += sp_bucket_x;
		if (f_kiana)
		{
			if (controllers[0].sThumbLY < -5000) bucket.y += sp_bucket_x;
			else if (controllers[0].sThumbLY >= 5000) bucket.y -= sp_bucket_x;
		}
        /*left and right triggers
        if (controllers[0].bLeftTrigger > 128) 
            bucket.x -= sp_bucket_x;
        else if (controllers[0].bRightTrigger > 128) 
            bucket.x += sp_bucket_x;

        //left and right D-PAD                                                                                                             
        if (controllers[0].wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) 
            bucket.x -= sp_bucket_x;
        else if (controllers[0].wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
            bucket.x += sp_bucket_x;

        //left and right shoulders
        if (controllers[0].wButtons & XINPUT_GAMEPAD_DPAD_LEFT) 
            bucket.x -= sp_bucket_x;
        else if (controllers[0].wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
            bucket.x += sp_bucket_x;*/
    }

    if (vibrating > 0)
    {
        vibrating++;
        if (vibrating > 20)
        {
            XInput_Vibrate(0, 0);
            vibrating = 0;
        }
    } 
    
    //保证篮子在屏幕内
    if (bucket.x < 0) bucket.x = 0;
    else if (bucket.x > SCREENW - 128) bucket.x = SCREENW - 128;
	if (bucket.y < 175) bucket.y = 175;
	else if (bucket.y > SCREENH - 128) bucket.y = SCREENH - 128;

    //接到炸弹1的判定
    int cx1 = (int)bomb1.x + 64;
    int cy1 = (int)bomb1.y + 64;

    if (cx1 > bucket.x && cx1 < bucket.x+128 && cy1 > bucket.y && cy1 < bucket.y+128)
    {
		
		if (bomb1.sq)
		{
			PlaySound(sound_kiana);
			sp_bomb1 /= 2;
			sp_bomb2 /= 2;
			f_kiana = 1;
			en1 = time(NULL);
		}
		else PlaySound(sound_bomb);
		if (f_kiana)
		{
			st_kiana = time(NULL);
			/*D3DXVECTOR3 pos3(0, 0, 0);
			spriteobj->Draw(num[(st_kiana - en2) % 10], NULL, NULL, &pos3, D3DCOLOR_XRGB(255, 255, 255));*/
			if (st_kiana - en2 >= 7 || st_kiana - en1 >= 7)
			{
				f_kiana = 0;
				sp_bomb1*=2;
				bucket.y = 630;
			}
		}
		bomb_count++;

		/*if (!x1) { en2 = st; x1 = 1; }
		if (bomb_count % 2) en1 = time(NULL);
		else en2 = time(NULL);

		sp_bomb1 +=abs(en1-en2)/20;
		if (sp_bomb1 >= 0.7) sp_bomb1 = 0.7;*/

		sp_bomb1 += sp_ex;
		if (sp_bomb1 < 0.1) sp_bomb1 = 0.1;
		//sp_bucket_x +=0.05;

        /*std::ostringstream os;
		os << APPTITLE << " [ time" << st_kiana - en2 << st_kiana - en1 << "]";
        string scoreStr = os.str();
        SetWindowText(window, scoreStr.c_str());*/

        //xbox控制器
        XInput_Vibrate(0, 65000);
        vibrating = 1;

        //重新生成炸弹
        bomb1.reset();
		//if (bomb1_surf) bomb1_surf->Release();
    }

	//接到炸弹2的判定
	int cx2 = (int)bomb2.x + 64;
	int cy2 = (int)bomb2.y + 64;
	time_t st2=0;
	if (cx2 > bucket.x && cx2 < bucket.x + 128 && cy2 > bucket.y && cy2 < bucket.y + 128)
	{
		if (bomb2.sq)
		{
			PlaySound(sound_kiana);
			sp_bomb2 /= 2.0;
			sp_bomb1 /= 2.0;
			f_kiana = 1;
			en2 = time(NULL);
		}
		if (f_kiana)
		{
			st_kiana = time(NULL);
			/*D3DXVECTOR3 pos3(300, 0, 0);
			spriteobj->Draw(num[(st_kiana - en2) % 10], NULL, NULL, &pos3, D3DCOLOR_XRGB(255, 255, 255));*/

			if (st_kiana - en2 >= 7 || st_kiana - en1 >= 7)
			{
				f_kiana = 0;
				sp_bomb2 *= 2;
				bucket.y = 630;
			}
		}
		else PlaySound(sound_bomb);	

		bomb_count++;

		/*if (!x2) {en4 = st;x2 = 1;}
		if (bomb_count % 2) en3 = time(NULL);
		else en4 = time(NULL);

		sp_bomb2 += abs(en3 - en4) / 20;
		sp_bucket_x += 0.05;
		if (sp_bomb2 >= 0.8) sp_bomb2 = 0.8;*/

		sp_bomb2 += sp_ex;
		if (sp_bomb2 < 0.1) sp_bomb2 = 0.1;
		/*std::ostringstream os;
		os << APPTITLE << " [ time" << st_kiana - en2 << st_kiana - en1 << "]" ;
		string scoreStr = os.str();
		SetWindowText(window, scoreStr.c_str());*/

		XInput_Vibrate(0, 65000);
		vibrating = 1;

		bomb2.reset();//重新生成炸弹
	}
	d3ddev->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO,&backbuffer);

    //start rendering
    if (d3ddev->BeginScene())
    {
		/*FontPrint(score_num, 60, 50, "This is the Arial 24 font printed with ID3DXFont");

		string text = "fuck you";
		score_num->spriteobj->DrawText(spriteobj,text.c_str(),text.length(),NULL,DT_WORDBREAK,NULL);
		spriteobj->End();*/

		spriteobj->Begin(D3DXSPRITE_ALPHABLEND);
		

		D3DXVECTOR3 pos1(bomb1.x, bomb1.y,0);
		if (bomb1.sq) spriteobj->Draw(bomb_ex_surf,NULL,NULL,&pos1,D3DCOLOR_XRGB(255,255,255));
		else spriteobj->Draw(bomb1_surf, NULL, NULL, &pos1, D3DCOLOR_XRGB(255,255,255));

		D3DXVECTOR3 pos2(bomb2.x, bomb2.y, 0);
		if(bomb2.sq) spriteobj->Draw(bomb_ex_surf, NULL, NULL, &pos2, D3DCOLOR_XRGB(255,255,255));
		else spriteobj->Draw(bomb2_surf, NULL, NULL, &pos2, D3DCOLOR_XRGB(255,255,255));

		D3DXVECTOR3 pos4(bucket.x, bucket.y, 0);
		spriteobj->Draw(bucket_surf, NULL, NULL, &pos4, D3DCOLOR_XRGB(255,255,255));

		int x=bomb_count,n = 0;
		while (x)
		{
			x /= 10;
			n++;
		}
		x = bomb_count;
		if(n==1)
		{
			D3DXVECTOR3 pos3(0, 0, 0);
			spriteobj->Draw(num[x%10], NULL, NULL, &pos3, D3DCOLOR_XRGB(255,255,255));
		}
		else if (n==2)
		{
			D3DXVECTOR3 pos3(75, 0, 0);
			spriteobj->Draw(num[x % 10], NULL, NULL, &pos3, D3DCOLOR_XRGB(255, 255, 255)); x /= 10;
			D3DXVECTOR3 pos4(0, 0, 0);
			spriteobj->Draw(num[x % 100], NULL, NULL, &pos4, D3DCOLOR_XRGB(255, 255, 255));
		}
		else if(n==3)
		{
			D3DXVECTOR3 pos3(85 * 2, 0, 0);
			spriteobj->Draw(num[x % 10], NULL, NULL, &pos3, D3DCOLOR_XRGB(255, 255, 255)); x /= 10;
			D3DXVECTOR3 pos4(85, 0, 0);
			spriteobj->Draw(num[x % 100], NULL, NULL, &pos4, D3DCOLOR_XRGB(255, 255, 255)); x /= 10;
			D3DXVECTOR3 pos5(0, 0, 0);
			spriteobj->Draw(num[x % 1000], NULL, NULL, &pos5, D3DCOLOR_XRGB(255, 255, 255));
		}

		d3ddev->StretchRect(surface, NULL, backbuffer, NULL, D3DTEXF_NONE);
		spriteobj->End();
        d3ddev->EndScene();
        d3ddev->Present(NULL, NULL, NULL, NULL);
    }
	/*if (Key_Down(DIK_SPACE))
	{
		space_count++;
		if (space_count % 2) sp_bomb1 = sp_bomb2 = sp_bucket_x = 0;
		else
		{
			sp_bomb1 = sp_bomb1_be;
			sp_bomb2 = sp_bomb2_be;
			sp_bucket_x = sp_bucket_x_st;
		}
	}
	*/
	if (Key_Down(DIK_SPACE)) sp_bomb1 = sp_bomb2 = 0.05;
		//gameover = true;
}

void Game_End()
{
	if (sound_bomb) delete sound_bomb;
	if(sound_bgm) delete sound_bgm;
	if (sound_kiana) delete sound_kiana;

	for(int i=0;i<10;i++)
	 num[10]->Release();
	if (bomb1_surf) bomb1_surf->Release();
	if (bomb2_surf) bomb2_surf->Release();
	if (bomb_ex_surf) bomb_ex_surf->Release();
    if (bucket_surf) bucket_surf->Release();
    DirectInput_Shutdown();
    Direct3D_Shutdown();
}