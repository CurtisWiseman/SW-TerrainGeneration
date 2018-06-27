#include "Player.h"
#include <dinput.h>
#include "GameData.h"

Player::Player(string _fileName, ID3D11Device* _pd3dDevice, IEffectFactory* _EF) : CMOGO(_fileName, _pd3dDevice, _EF)
{
	//any special set up for Player goes here
	m_fudge = Matrix::CreateRotationY(XM_PI);

	m_pos.y = 10.0f;

	SetDrag(0.7);
	SetPhysicsOn(true);
}

Player::~Player()
{
	//tidy up anything I've created
}


void Player::Tick(GameData* _GD)
{
	switch (_GD->m_GS)
	{
	case GS_PLAY_MAIN_CAM:
	{
		{
			//MOUSE CONTROL SCHEME HERE
			/*float speed = 10.0f;
			m_acc.x += speed * _GD->m_mouseState->lX;
			m_acc.z += speed * _GD->m_mouseState->lY;
			break;*/
		}
	}
	case GS_PLAY_TPS_CAM:
	{
		//TURN AND FORWARD CONTROL HERE
		Vector3 forwardMove = 40.0f * Vector3::Forward;
		Matrix rotMove = Matrix::CreateRotationY(m_yaw);
		forwardMove = Vector3::Transform(forwardMove, rotMove);
		if (_GD->m_keyboardState[DIK_W] & 0x80)
		{
			m_acc += forwardMove;
		}
		if (_GD->m_keyboardState[DIK_S] & 0x80)
		{
			m_acc -= forwardMove;
		}
		break;
	}
	}

	//change orinetation of player
	float rotSpeed = 2.0f * _GD->m_dt;
	if (_GD->m_keyboardState[DIK_A] & 0x80)
	{
		m_yaw += rotSpeed;
	}
	if (_GD->m_keyboardState[DIK_D] & 0x80)
	{
		m_yaw -= rotSpeed;
	}
	if (_GD->m_keyboardState[DIK_Q] & 0x80)
	{
		m_pitch += rotSpeed;
	}
	if (_GD->m_keyboardState[DIK_E] & 0x80)
	{
		m_pitch -= rotSpeed;
	}

	//move player up and down
	if (_GD->m_keyboardState[DIK_R] & 0x80)
	{
		m_acc.y += 40.0f;
	}

	if (_GD->m_keyboardState[DIK_F] & 0x80)
	{
		m_acc.y -= 40.0f;
	}

	if (m_pitch >= 0.5 * XM_PI)
	{
		m_pitch = 0.5 * XM_PI - 0.001f;
	}
	if (m_pitch <= -0.5 * XM_PI)
	{
		m_pitch = -0.5 * XM_PI + 0.001f;
	}

	//apply my base behaviour
	CMOGO::Tick(_GD);
}

void Player::Draw(DrawData* _DD)
{

}