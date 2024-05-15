
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
Vector2 currentPosition;
float currentScale = 10.f;

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float moveSpeed = 100.f;
	static float scaleMin = 5.f;
	static float scaleMax = 20.f;

	// ******************** sin함수의 주기성을 활용하여 하트가 커졌다 작아졌다를 반복하는 애니메이션 구현*********************************
	// 이때, sin함수는 y=sin(2pi*t/T)로, 시간 도메인에서의 sin함수를 사용한다.
	static float duration = 1.5f;		// 애니메이션의 주기. T
	static float elapsedTime = 0.f;		// 주기함수의 현재 시간. t

	Vector2 inputVector = Vector2(input.GetAxis(InputAxis::XAxis), input.GetAxis(InputAxis::YAxis)).GetNormalize();
	Vector2 deltaPosition = inputVector * moveSpeed * InDeltaSeconds;

	// 경과 시간과 sin 한수를 활용한 [0, 1] 값의 생성
	elapsedTime += InDeltaSeconds;									// 시간의 흐름
	elapsedTime = Math::FMod(elapsedTime, duration);				// sin함수는 주기함수이기 때문에, t가 주기를 넘었을 때, 다시 0부터 시작해도 함수는 연속된다.(
	float currentRad = (elapsedTime / duration) * Math::TwoPI;		// 현재 각도 = (2pi*t/T) 표현.
	float alpha = (sinf(currentRad) + 1) * 0.5f;					// sin은 [-1, 1]의 값을 가지며, 현재 애니메이션 의도는 뒤집어지는 것이 아닌 작아졌다 커지는 것이기 때문에, 
																	// sin함수의 값을 [0, 1]로 변환할 필요가 있다.
																	// 때문에, + 1을 통해 [0,2]로 증가시킨 다음, 모든 값들을 절반으로 나눠 [0, 1]이 되도록 한다.

	// 물체의 최종 상태 설정
	currentPosition += deltaPosition;
	currentScale = Math::Lerp(scaleMin, scaleMax, alpha);			// Lerp는 선형 보간을 시켜주는 함수다.
																	// 세번째 인자값이 0이면 scaleMin, 1이면 scaleMax, 그 사이 값이라면 scaleMin과 scaleMax 값 사이의 적절한 보간 값을 반환한다.
}

// 렌더링 로직을 담당하는 함수
void SoftRenderer::Render2D()
{
	// 렌더링 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 배경에 격자 그리기
	DrawGizmo2D();

	// 렌더링 로직의 로컬 변수
	float rad = 0.f;
	static float increment = 0.001f;
	static std::vector<Vector2> hearts;
	HSVColor hsv(0.f, 1.f, 0.85f);

	// 하트를 구성하는 점 생성
	if (hearts.empty())
	{
		for (rad = 0.f; rad < Math::TwoPI; rad += increment)
		{
			float sin = sinf(rad);
			float cos = cosf(rad);
			float cos2 = cosf(2 * rad);
			float cos3 = cosf(3 * rad);
			float cos4 = cosf(4 * rad);
			float x = 16.f * sin * sin * sin;
			float y = 13 * cos - 5 * cos2 - 2 * cos3 - cos4;
			hearts.push_back(Vector2(x, y));
		}
	}

	// 각 값을 초기화한 후 색상을 증가시키면서 점에 대응
	rad = 0.f;
	for (auto const& v : hearts)
	{
		hsv.H = rad / Math::TwoPI;
		r.DrawPoint(v * currentScale + currentPosition, hsv.ToLinearColor());
		rad += increment;
	}

	// 현재 위치와 스케일을 화면에 출력
	r.PushStatisticText(std::string("Position : ") + currentPosition.ToString());
	r.PushStatisticText(std::string("Scale : ") + std::to_string(currentScale));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
