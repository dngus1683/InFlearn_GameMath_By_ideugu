
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
Vector2 lightPosition(200.f, 0.f);											// 광원 위치.
LinearColor lightColor;														// 빛 색상.
Vector2 circlePosition;														// 원 위치.

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float duration = 20.f;
	static float elapsedTime = 0.f;
	static float currentDegree = 0.f;
	static float lightDistance = 200.f;
	static HSVColor lightHSVColor;

	// 경과 시간에 따른 현재 각과 이를 사용한 [0,1]값의 생성
	elapsedTime += InDeltaSeconds;
	elapsedTime = Math::FMod(elapsedTime, duration);						// mod 계산을 통해 경과 시간이 duration을 넘지 않도록 함. 
	float currentRad = (elapsedTime / duration) * Math::TwoPI;				// 경과 시간에 비례하여 현재 각 계산. == radian 값.
	float alpha = (sinf(currentRad) + 1) * 0.5f;							// sin함수를 활용하여 현재 각에 따른 [0,1]사이의 값 얻음.

	// [0,1]을 활용해 주기적으로 크기를 반복하기
	currentDegree = Math::Lerp(0.f, 360.f, alpha);							// Lerp를 통해, 위에서 구한 [0,1]값에 따른 currentDegree 값 보간. == (radian => degree) 값 변환

	// 광원의 좌표와 색상
	float sin = 0.f;
	float cos = 0.f;
	Math::GetSinCos(sin, cos, currentRad);
	lightPosition = Vector2(cos, sin) * lightDistance;						// 각도에 따른 데카르트 좌표값({cos, sin})을 얻고 이에 길이를 곱해 최종 광원 위치 업데이트.

	lightHSVColor.H = currentRad * Math::InvPI * 0.5f;						// 현재 각도를 360도로 나눠 [0,1] 값으로 정규화.
	lightColor = lightHSVColor.ToLinearColor();								// HSV -> RGBA.
}

// 렌더링 로직을 담당하는 함수
void SoftRenderer::Render2D()
{
	// 렌더링 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 렌더링 로직의 로컬 변수
	static std::vector<Vector2> light;
	static float lightRadius = 10.f;
	static std::vector<Vector2> circle;
	static float circleRadius = 50.f;

	// 광원을 표현하는 구체
	// 원 그리는 함수와 동일.
	if (light.empty())
	{
		float lightRadius = 10.f;
		for (float x = -lightRadius; x <= lightRadius; ++x)
		{
			for (float y = -lightRadius; y <= lightRadius; ++y)
			{
				Vector2 target(x, y);
				float sizeSquared = target.SizeSquared();
				float rr = lightRadius * lightRadius;
				if (sizeSquared < rr)
				{
					light.push_back(target);
				}
			}
		}
	}

	// 빛을 받는 물체
	// 원 그리는 함수와 동일.
	if (circle.empty())
	{
		for (float x = -circleRadius; x <= circleRadius; ++x)
		{
			for (float y = -circleRadius; y <= circleRadius; ++y)
			{
				Vector2 target(x, y);
				float sizeSquared = target.SizeSquared();
				float rr = circleRadius * circleRadius;
				if (sizeSquared < rr)
				{
					circle.push_back(target);
				}
			}
		}
	}

	// 광원 그리기
	static float lightLineLength = 50.f;
	r.DrawLine(lightPosition, lightPosition - lightPosition.GetNormalize() * lightLineLength, lightColor);			// 광원에서 물체를 향한 방향으로 50만큼의 길이의 선분 그리기.
	for (auto const& v : light)
	{
		r.DrawPoint(v + lightPosition, lightColor);
	}

	// 광원을 받는 구체의 모든 픽셀에 NdotL을 계산해 음영을 산출하고 이를 최종 색상에 반영
	for (auto const& v : circle)
	{
		Vector2 n = (v - circlePosition).GetNormalize();						// 원의 중심에서 원을 이루는 점을 바라보는 뱡향으로의 법선 벡터.
		Vector2 l = (lightPosition - v).GetNormalize();							// 원을 이루는 점에서 광원을 바라보는 방향의 광원 벡터.
		float shading = Math::Clamp(n.Dot(l), 0.f, 1.f);						// 두 벡터가 이루는 각에 따라 해당 물체의 해당 위치에 빛이 얼마나 받는지를 내적값(cos값)으로 계산.
		r.DrawPoint(v, lightColor*shading);										// cos값([0,1])에 따라 광원의 세기 조절.
	}

	// 현재 조명의 위치를 화면에 출력
	r.PushStatisticText(std::string("Position : ") + lightPosition.ToString());
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
