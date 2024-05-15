
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
Vector2 CurrentPosition;	// 하트 중심점 위치
float CurrentScale = 10.f;	// 하트의 크기 배율

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float MoveSpeed = 100.f;
	static float ScaleMin = 5.f;
	static float ScaleMax = 20.f;
	static float ScaleSpeed = 20.f;

	Vector2 InputVector = Vector2(input.GetAxis(InputAxis::XAxis), input.GetAxis(InputAxis::YAxis)).GetNormalize();
	Vector2 DeltaPosition = InputVector * MoveSpeed * InDeltaSeconds;
	float DeltaScale = input.GetAxis(InputAxis::ZAxis) * ScaleSpeed * InDeltaSeconds;

	// 물체의 최종 상태 설정
	CurrentPosition += DeltaPosition;
	CurrentScale = Math::Clamp(CurrentScale + DeltaScale, ScaleMin, ScaleMax);	// Clamp 함수는, 주어진 변수가 지정해준 최솟값과 최댓값을 넘기지 못하도록 집어주는 함수다.
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

	// 하트를 구성하는 점 생성
	if (hearts.empty())
	{
		for (rad = 0.f; rad < Math::TwoPI; rad += increment)	// 0도에서 한 바퀴 돌아 360도까지 계산.
		{
			// 하트 방정식
			// x와 y를 구하기.
			float sin = sinf(rad);
			float cos[4] = { cosf(rad), cosf(2*rad), cosf(3 * rad) , cosf(4 * rad) };
			float x = 16 * sin * sin * sin;	// 하트 방정식 x 식
			float y = 13 * cos[0] - 5 * cos[1] - 2 * cos[2] - cos[3]; // 하트 방정식 y식
			hearts.push_back(Vector2(x, y));	// 위 식을 통해 도출된 좌표값을 하트 컨테이너에 추가.
		}
	}

	for (auto const& v : hearts)
	{
		r.DrawPoint(v * CurrentScale  + CurrentPosition, LinearColor::Blue);	// 위치를 이동시킬 때는 모양을 건드는 것이 아닌, 모든 좌표값들이 동일하게 위치값만 바뀌는 것이기 때문에 더한다.
																				// scale이나 rotate와 같이 모양과 자세를 건드는 것은 모든 좌표값들에 동일한 값이 계산되지 않고, 선형 변환의 식에 따라 변하기 때문에 곱셈으로 적용한다.
	}

	r.PushStatisticText("Position : " + CurrentPosition.ToString());
	r.PushStatisticText("Scale : " + std::to_string(CurrentScale));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
