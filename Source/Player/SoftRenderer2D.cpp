
#include "Precompiled.h"
#include "SoftRenderer.h"
#include <random>
using namespace CK::DD;

// 격자를 그리는 함수
void SoftRenderer::DrawGizmo2D()
{
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 그리드 색상
	LinearColor gridColor(LinearColor(0.8f, 0.8f, 0.8f, 0.3f));

	// 뷰의 영역 계산
	Vector2 viewPos = g.GetMainCamera().GetTransform().GetPosition();
	Vector2 extent = Vector2(_ScreenSize.X * 0.5f, _ScreenSize.Y * 0.5f);

	// 좌측 하단에서부터 격자 그리기
	int xGridCount = _ScreenSize.X / _Grid2DUnit;
	int yGridCount = _ScreenSize.Y / _Grid2DUnit;

	// 그리드가 시작되는 좌하단 좌표 값 계산
	Vector2 minPos = viewPos - extent;
	Vector2 minGridPos = Vector2(ceilf(minPos.X / (float)_Grid2DUnit), ceilf(minPos.Y / (float)_Grid2DUnit)) * (float)_Grid2DUnit;
	ScreenPoint gridBottomLeft = ScreenPoint::ToScreenCoordinate(_ScreenSize, minGridPos - viewPos);

	for (int ix = 0; ix < xGridCount; ++ix)
	{
		r.DrawFullVerticalLine(gridBottomLeft.X + ix * _Grid2DUnit, gridColor);
	}

	for (int iy = 0; iy < yGridCount; ++iy)
	{
		r.DrawFullHorizontalLine(gridBottomLeft.Y - iy * _Grid2DUnit, gridColor);
	}

	ScreenPoint worldOrigin = ScreenPoint::ToScreenCoordinate(_ScreenSize, -viewPos);
	r.DrawFullHorizontalLine(worldOrigin.Y, LinearColor::Red);
	r.DrawFullVerticalLine(worldOrigin.X, LinearColor::Green);
}

// 게임 오브젝트 목록


// 최초 씬 로딩을 담당하는 함수
void SoftRenderer::LoadScene2D()
{
	// 최초 씬 로딩에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();

}

// 게임 로직과 렌더링 로직이 공유하는 변수
float fovAngle = 60.f;							// 플레이어의 시야각
Vector2 playerPosition(0.f, 0.f);				// 플레이어의 위치
LinearColor playerColor = LinearColor::Gray;	// 플레이어의 색상
Vector2 targetPosition(0.f, 100.f);				// 목표물의 위치
LinearColor targetColor = LinearColor::Blue;	// 목표물의 색상

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float moveSpeed = 100.f;
	static std::random_device rd;												// random_device를 통해 시드값을 얻음.
	static std::mt19937 mt(rd());												// random_device를 통해 얻은 시드값을 통해 난수 생성 엔진을 초기화.
	static std::uniform_real_distribution<float> randomPosX(-300.f, 300.f);		// -300부터 300까지 균등하게 생성되도록 하는 실수 난수 생성 균등 분포 정의.
	static std::uniform_real_distribution<float> randomPosY(-200.f, 200.f);		// -200부터 200까지 균등하게 생성되도록 하는 실수 난수 생성 균등 분포 정의.
	static float duration = 3.f;
	static float elapsedTime = 0.f;
	static Vector2 targetStart = targetPosition;								// 목표물 시작 위치 설정.
	static Vector2 targetDestination = Vector2(randomPosX(mt), randomPosY(mt));	// 목표물의 랜덤 목표 위치.

	// 시야각의 cos 값은 최초 1회만 계산해 보관한다.
	static float halfFovCos = cosf(Math::Deg2Rad(fovAngle * 0.5f));				// 플레이어의 정면과 플레이어 기준 목표물의 방향이 이루는 각이, 플레이어 시야각의 절반 각과 비교되기 때문.

	elapsedTime = Math::Clamp(elapsedTime + InDeltaSeconds, 0.f, duration);		// 경과 시간이 duration을 넘지 않도록 clamp함.

	// 지정한 시간이 경과하면 새로운 이동 지점을 랜덤하게 설정
	if (elapsedTime == duration)
	{
		targetStart = targetDestination;
		targetPosition = targetDestination;
		targetDestination = Vector2(randomPosX(mt), randomPosY(mt));

		elapsedTime = 0.f;
	}
	else // 비율에 따라 목표지점까지 선형보간하면서 이동
	{
		float ratio = elapsedTime / duration;

		// Lerp함수를 통해 선형 보간 구현.
		targetPosition = Vector2(
			Math::Lerp(targetStart.X, targetDestination.X, ratio),
			Math::Lerp(targetStart.Y, targetDestination.Y, ratio)
		);
	}

	Vector2 inputVector = Vector2(input.GetAxis(InputAxis::XAxis), input.GetAxis(InputAxis::YAxis)).GetNormalize();
	Vector2 deltaPosition = inputVector * moveSpeed * InDeltaSeconds;

	Vector2 f = Vector2::UnitY;										// 플레이어 시야방향. 해당 실습에는 시야방향을 y축으로 고정.
	Vector2 v = (targetPosition - playerPosition).GetNormalize();	// 플레이어 위치에서 목표물 위치로 가는 방향의 단위 벡터.

	// 물체의 최종 상태 설정
	if (v.Dot(f) >= halfFovCos)										// 플레이어 시야 벡터와, 목표물 방향의 벡터 간의 내적값이 (시야각 % 2) 보다 클 경우 == 시야각 안에 목표물이 있는 경우.
	{
																	// 플레이어와 목표물의 색상을 Red로.
		playerColor = LinearColor::Red;
		targetColor = LinearColor::Red;
	}
	else
	{
																	// 내적값이 (시야각 % 2) 보다 작을 경우 == 시야각 외부에 목표물이 있는 경우.
																	// 플레이어와 목표물 색상을 각각 Gray, Blue로.
		playerColor = LinearColor::Gray;			
		targetColor = LinearColor::Blue;
	}
	playerPosition += deltaPosition;
}

// 렌더링 로직을 담당하는 함수
void SoftRenderer::Render2D()
{
	// 렌더링 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 렌더링 로직의 로컬 변수
	static float radius = 5.f;									// 플레이어와 목표물을 표현할 원의 반지름.
	static std::vector<Vector2> sphere;							// 플레이어와 목표물을 표현할 원.
	static float sightLength = 300.f;							// 시야각 보조선 길이.


	// 반지름을 기준으로 원의 점들 정의.
	if (sphere.empty())
	{
		for (float x = -radius; x <= radius; ++x)
		{
			for (float y = -radius; y <= radius; ++y)
			{
				Vector2 target(x, y);
				float sizeSquared = target.SizeSquared();
				float rr = radius * radius;
				if (sizeSquared < rr)
				{
					sphere.push_back(target);
				}
			}
		}
	}

	// 플레이어 렌더링. 
	float halfFovSin = 0.f, halfFovCos = 0.f;
	Math::GetSinCos(halfFovSin, halfFovCos, fovAngle * 0.5f);				// 시야각 보조선을 그리기 위해, 시야각 절반의 sin, cos 정의.

	r.DrawLine(playerPosition, playerPosition + Vector2(sightLength * halfFovSin, sightLength * halfFovCos), playerColor);	// (sin, cos)이 y축 기준 우측상단으로 향하는 시야각 보조선의 단위벡터
	r.DrawLine(playerPosition, playerPosition + Vector2(-sightLength * halfFovSin, sightLength * halfFovCos), playerColor); // (-sin, cos)이 y축 기준 좌측상단으로 향하는 시야각 보조선의 단위벡터
	r.DrawLine(playerPosition, playerPosition + Vector2::UnitY * sightLength * 0.2f, playerColor);							// 플레이어 정면 벡터. y축 고정.
	for (auto const& v : sphere)
	{
		r.DrawPoint(v + playerPosition, playerColor);
	}

	// 타겟 렌더링
	for (auto const& v : sphere)
	{
		r.DrawPoint(v + targetPosition, targetColor);
	}

	// 주요 정보 출력
	r.PushStatisticText(std::string("Player Position : ") + playerPosition.ToString());
	r.PushStatisticText(std::string("Target Position : ") + targetPosition.ToString());
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
