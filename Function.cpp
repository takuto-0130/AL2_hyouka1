﻿#include "Function.h"


void FindVertex(Vector2 center, float width, float height, Vector2* lt, Vector2* rt, Vector2* lb, Vector2* rb)
{
	*lt = { center.x - width / 2, center.y - height / 2 };
	*rt = { (center.x + width / 2 - 1), center.y - height / 2 };
	*lb = { center.x - width / 2, (center.y + height / 2 - 1) };
	*rb = { (center.x + width / 2 - 1), (center.y + height / 2 - 1) };
}

void MoveLimit(Player& a) {
	if (a.worldPos.x < 0 + a.len.x / 2) {
		a.worldPos.x = 0 + a.len.x / 2;
	}
	if (a.worldPos.x > kWorldEnd) {
		a.worldPos.x = float(kWorldEnd);
	}
	if (a.worldPos.y < 0) {
		a.worldPos.y = 0;
	}
	if (a.worldPos.y > 800) {
		a.worldPos.y = 800;
	}
}

Vector2 Normalize(Vector2 v) {
	float denominator = sqrtf(v.x * v.x + v.y * v.y);
	Vector2 result = { 0,0 };
	if (denominator != 0) {
		result = { sqrtf(v.x * v.x) / denominator, sqrtf(v.y * v.y) / denominator };
	}
	return result;
}


void MovePlayer(Player& a) {
	a.velocity = {
		(a.moveSpeed.x * a.direction.x/* * a.normalize.x*/),
		(a.moveSpeed.y * a.direction.y/* * a.normalize.y*/)
	};
	a.worldPos = {
		a.worldPos.x + a.velocity.x,
		a.worldPos.y + a.velocity.y
	};
}

void winMapRect(WindowMap& a, int x, int y) {
	a.localRect[y][x].leftTop = { float(-a.tileSize / 2.0f), float(-a.tileSize / 2.0f) };
	a.localRect[y][x].rightTop = { float(a.tileSize / 2.0f), float(-a.tileSize / 2.0f) };
	a.localRect[y][x].leftBottom = { float(-a.tileSize / 2.0f), float(a.tileSize / 2.0f) };
	a.localRect[y][x].rightBottom = { float(a.tileSize / 2.0f), float(a.tileSize / 2.0f) };
	a.pos[y][x] = { float(a.tileSize * x) + float(a.tileSize / 2.0f), float(a.tileSize * y) + float(a.tileSize / 2.0f) };
};

void DrawQuadVertex(Vertex a, int texHandle, Vector2 texLeftTop, Vector2 texLength, unsigned int color) {
	Novice::DrawQuad(int(a.leftTop.x), int(a.leftTop.y), int(a.rightTop.x), int(a.rightTop.y), int(a.leftBottom.x), int(a.leftBottom.y), int(a.rightBottom.x), int(a.rightBottom.y),
		int(texLeftTop.x), int(texLeftTop.y), int(texLength.x), int(texLength.y), texHandle, color);
}

Vector2 Transform(Vector2 vector, Matrix3x3 matrix) {
	Vector2 result;
	result.x = vector.x * matrix.m[0][0] + vector.y * matrix.m[1][0] + 1.0f * matrix.m[2][0];
	result.y = vector.x * matrix.m[0][1] + vector.y * matrix.m[1][1] + 1.0f * matrix.m[2][1];
	float w = vector.x * matrix.m[0][2] + vector.y * matrix.m[1][2] + 1.0f * matrix.m[2][2];
	assert(w != 0.0f);
	result.x /= w;
	result.y /= w;
	return result;
}

Matrix3x3 MakeAffineMatrix(Vector2 scale, float theta, Vector2 transrate) {
	Matrix3x3 m1;
	m1.m[0][0] = scale.x * cosf(theta);  m1.m[0][1] = scale.x * sinf(theta); m1.m[0][2] = 0;
	m1.m[1][0] = scale.y * -sinf(theta); m1.m[1][1] = scale.y * cosf(theta); m1.m[1][2] = 0;
	m1.m[2][0] = transrate.x;			 m1.m[2][1] = transrate.y;			 m1.m[2][2] = 1;
	return m1;
}

Matrix3x3 MakeRotateMatrix(float theta) {
	Matrix3x3 m1;
	m1.m[0][0] = cosf(theta);  m1.m[0][1] = sinf(theta); m1.m[0][2] = 0;
	m1.m[1][0] = -sinf(theta); m1.m[1][1] = cosf(theta); m1.m[1][2] = 0;
	m1.m[2][0] = 0;			   m1.m[2][1] = 0;			 m1.m[2][2] = 1;
	return m1;
}

Vector2 NormalizeDead(Vector2 v) {
	float denominator = sqrtf(v.x * v.x + v.y * v.y);
	Vector2 result = { 0,0 };
	if (denominator != 0) {
		result = { v.x / denominator, v.y / denominator };
	}
	return result;
}

void easeOut(ParticleDead& a, EmitterDead b) {
	float ease = 0;
	for (int i = 0; i < kDeadParticleNumber; i++)
	{
		ease = powf(a.life[i] / b.life, 4);
		a.position[i].x = a.startPos[i].x + ((b.position.x - a.startPos[i].x) * ease);
		a.position[i].y = a.startPos[i].y + ((b.position.y - a.startPos[i].y) * ease);
	}
}

void LoadMap(MapChipNum& map, const char* dirName) {
	int c;//文字を格納する変数
	int retu = 0;
	int gyou = 0;
	char buf[10];//文字列を格納するバッファ
	memset(buf, 0, sizeof(buf));//←↑2桁以上のマップチップ番号を読み込むときに必要
	bool eofFlag = 0;
	FILE* fp;//ファイルポインタを宣言
	errno_t error;//ファイルが読み取れているかどうかa
	error = fopen_s(&fp, dirName, "r");//ファイルを開いてポインタに結び付ける
	if (error != 0) {//ファイルが読み取れていないときに動かさないようにする
	}
	else {
		while (1) {
			while (1) {
				c = fgetc(fp);//文字を読み込みcに格納
				if (c == EOF) {
					int num = atoi(buf);//文字列をint型に変換
					map.mapData[gyou][retu] = num;//マップ番号の割り振り
					memset(buf, 0, sizeof(buf));//バッファの値をリセット
					eofFlag = 1;//EndOfFileの時にループを抜ける
					break;
				}
				if (c != ',' && c != '\n') {
					strcat_s(buf, (const char*)&c);//cが区切りや改行でなければバッファの数字に連結(２桁以上で必要)
				}
				else {
					int num = atoi(buf);//文字列をint型に変換
					map.mapData[gyou][retu] = num;//マップ番号の割り振り
					memset(buf, 0, sizeof(buf));//バッファの値をリセット
					break;//区切りか改行ならループを抜ける
				}
			}
			if (eofFlag != 0) {
				break;
			}
			if (c == ',') {
				retu++;
			}
			if (c == '\n') {
				gyou++;
				retu = 0;
			}
		}
		fclose(fp);//ファイルを閉じる
	}

}

void CollisionBlock(MapChipNum& map, Player& player, const float resist, int& stopFrame, int& stopTimer, BrockEmitter* emitter, uint32_t& breakCount) {

	if (map.mapData[int(player.lt.y) / blockSize][int(player.lt.x) / blockSize] == 3) {
		map.mapData[int(player.lt.y) / blockSize][int(player.lt.x) / blockSize] = 0;
		player.resistance += resist;
		stopTimer = stopFrame;
		emitter->Emit({ float((int(player.lt.x)) + blockSize / 2), float((int(player.lt.y)) - blockSize / 2) });
		breakCount++;
	}

	if (map.mapData[int(player.rt.y) / blockSize][int(player.rt.x) / blockSize] == 3) {
		map.mapData[int(player.rt.y) / blockSize][int(player.rt.x) / blockSize] = 0;
		player.resistance += resist;
		stopTimer = stopFrame;
		emitter->Emit({ float((int(player.rt.x)) - blockSize / 2), float((int(player.rt.y)) - blockSize / 2) });
		breakCount++;
	}

	if (map.mapData[int(player.lb.y) / blockSize][int(player.lb.x) / blockSize] == 3) {
		map.mapData[int(player.lb.y) / blockSize][int(player.lb.x) / blockSize] = 0;
		player.resistance += resist;
		stopTimer = stopFrame;
		emitter->Emit({ float((int(player.lb.x)) + blockSize / 2), float((int(player.lb.y)) + blockSize / 2) });
		breakCount++;
	}

	if (map.mapData[int(player.rb.y) / blockSize][int(player.rb.x) / blockSize] == 3) {
		map.mapData[int(player.rb.y) / blockSize][int(player.rb.x) / blockSize] = 0;
		player.resistance += resist;
		stopTimer = stopFrame;
		emitter->Emit({ float((int(player.rb.x)) - blockSize / 2), float((int(player.rb.y)) + blockSize / 2) });
		breakCount++;
	}
}

void PlayerMapCollision(MapChipNum& map, Player& player) {
	if (map.mapData[int(player.lb.y + (player.moveSpeed.y * player.direction.y)) / blockSize][int(player.lb.x) / blockSize] == 0 &&
		map.mapData[int(player.rb.y + (player.moveSpeed.y * player.direction.y)) / blockSize][int(player.rb.x) / blockSize] == 0) {
		player.isAir = 1;
	}
	if (player.isAir == 0)
	{
		if (player.direction.x > 0)
		{
			if (map.mapData[int(player.rt.y) / blockSize][int(player.rt.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1 ||
				map.mapData[int(player.rb.y - player.tempVelo.y) / blockSize][int(player.rb.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1) {
				player.worldPos.x = float(int(player.rt.x + (player.moveSpeed.x * player.direction.x)) / blockSize) * blockSize - (player.len.x + player.sizeChange.x) / 2;
				player.direction.x = 0;
			}
		}
		if (player.direction.x < 0)
		{
			if (map.mapData[int(player.lt.y) / blockSize][int(player.lt.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1 ||
				map.mapData[int(player.lb.y - player.tempVelo.y) / blockSize][int(player.lb.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1) {
				player.worldPos.x = float(int(player.lb.x + (player.moveSpeed.x * player.direction.x)) / blockSize) * blockSize + (player.len.x + player.sizeChange.x) / 2 + blockSize;
				player.direction.x = 0;
			}
		}
		if (map.mapData[int(player.lt.y + (player.moveSpeed.y * player.direction.y)) / blockSize][int(player.lt.x) / blockSize] == 1 ||
			map.mapData[int(player.rt.y + (player.moveSpeed.y * player.direction.y)) / blockSize][int(player.rt.x) / blockSize] == 1) {
			player.worldPos.y = float(int(player.rt.y + (player.moveSpeed.y * player.direction.y)) / blockSize) * blockSize + (player.len.y + player.sizeChange.y) / 2 + blockSize;
			player.direction.y = 0;
			player.tempVelo.y = 0;
		}
		if (map.mapData[int(player.lb.y + (player.moveSpeed.y * player.direction.y) + player.tempVelo.y) / blockSize][int(player.lb.x) / blockSize] == 1 ||
			map.mapData[int(player.rb.y + (player.moveSpeed.y * player.direction.y) + player.tempVelo.y) / blockSize][int(player.rb.x) / blockSize] == 1) {
			player.worldPos.y = float(int(player.rb.y + (player.moveSpeed.y * player.direction.y)) / blockSize) * blockSize - (player.len.y + player.sizeChange.y) / 2;
			player.direction.y = 0;
			player.tempVelo.y = 0;
			player.isAir = 0;
		}
	}
	else
	{
		if (player.direction.x > 0)
		{
			if (map.mapData[int(player.rt.y) / blockSize][int(player.rt.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1 ||
				map.mapData[int(player.rb.y) / blockSize][int(player.rb.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1) {
				player.worldPos.x = float(int(player.rt.x + (player.moveSpeed.x * player.direction.x)) / blockSize) * blockSize - (player.len.x + player.sizeChange.x) / 2;
				player.direction.x = 0;
			}
		}
		if (player.direction.x < 0)
		{
			if (map.mapData[int(player.lt.y) / blockSize][int(player.lt.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1 ||
				map.mapData[int(player.lb.y) / blockSize][int(player.lb.x + (player.moveSpeed.x * player.direction.x)) / blockSize] == 1) {
				player.worldPos.x = float(int(player.lb.x + (player.moveSpeed.x * player.direction.x)) / blockSize) * blockSize + (player.len.x + player.sizeChange.x) / 2 + blockSize;
				player.direction.x = 0;
			}
		}
		if (map.mapData[int(player.lt.y + (player.moveSpeed.y * player.direction.y)) / blockSize][int(player.lt.x) / blockSize] == 1 ||
			map.mapData[int(player.rt.y + (player.moveSpeed.y * player.direction.y)) / blockSize][int(player.rt.x) / blockSize] == 1) {
			player.worldPos.y = float(int(player.rt.y/* + (player.moveSpeed.y * player.direction.y)*/) / blockSize) * blockSize + (player.len.y + player.sizeChange.y) / 2;
			player.direction.y = 0;
			player.tempVelo.y = 0;
			player.velocity.y = 0;
		}
		if (map.mapData[int(player.lb.y + (player.moveSpeed.y * player.direction.y) + player.tempVelo.y) / blockSize][int(player.lb.x) / blockSize] == 1 ||
			map.mapData[int(player.rb.y + (player.moveSpeed.y * player.direction.y) + player.tempVelo.y) / blockSize][int(player.rb.x) / blockSize] == 1) {
			player.worldPos.y = float(int(player.rb.y + (player.moveSpeed.y * player.direction.y) + player.tempVelo.y) / blockSize) * blockSize - (player.len.y + player.sizeChange.y) / 2;
			player.direction.y = 0;
			player.tempVelo.y = 0;
			player.isAir = 0;
		}
	}
}

#pragma region // Vector2
Vector2 operator+(const Vector2& a, const Vector2& b) {
	Vector2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

Vector2 operator-(const Vector2& a, const Vector2& b) {
	Vector2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

Vector2 operator-(const Vector2& a) {
	Vector2 result;
	result.x = -a.x;
	result.y = -a.y;
	return result;
}

Vector2 operator*(const Vector2& v, const float& s) {
	Vector2 result;
	result.x = v.x * s;
	result.y = v.y * s;
	return result;
}

Vector2 operator*(const float& s, const Vector2& v) {
	Vector2 result;
	result.x = v.x * s;
	result.y = v.y * s;
	return result;
}

Vector2 operator/(const Vector2& v, const float& s) {
	Vector2 result;
	result.x = v.x / s;
	result.y = v.y / s;
	return result;
}

Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t) {
	return v1 + t * (v2 - v1);
}
#pragma endregion



void TimeDisplay(const uint32_t time, Timedisp& a) {

	int byou = int(time / 60);
	a.seconds[0] = byou / 10;
	byou = byou % 10;

	a.seconds[1] = byou;
}

void ScoreDisplay(const uint32_t score, ScoreDisp& a) {

	uint32_t byou = score;
	a.num[0] = byou / 100;
	byou = byou % 100;

	a.num[1] = byou / 10;
	byou = byou % 10;

	a.num[2] = byou;
}