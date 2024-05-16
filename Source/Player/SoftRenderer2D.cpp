
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
Vector2 point(0.f, 250.f);									// 점의 위치.
Vector2 lineStart(-400.f, 0.f);								// 점이 투영할 선의 시작점
Vector2 lineEnd(400.f, 0.f);								// 점이 투영할 선의 끝점.

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float duration = 6.f;
	static float elapsedTime = 0.f;
	static float currentDegree = 0.f;
	static float rotateSpeed = 180.f;
	static float distance = 250.f;
	static std::random_device rd;											// 난수를 생성할 때 사용될 시드값을 얻음.
	static std::mt19937 mt(rd());											// rt를 통해 생성한 시드값을 통해 난수를 생성하는 난수 생성 엔진 초기화.
	static std::uniform_real_distribution<float> randomY(-200.f, 200.f);	// 200부터 200까지 균등하게 생성되도록 하는 실수 난수 생성 균등 분포 정의.
																			// 이를 활용하여, 선을 구성하는 두 점의 y값 랜덤 생성.
	elapsedTime = Math::Clamp(elapsedTime + InDeltaSeconds, 0.f, duration);
	if (elapsedTime == duration)
	{
		// 난수 생성 함수를 통해, 선을 구성하는 두 점의 y값 랜덤 생성.
		lineStart = Vector2(-400.f, randomY(mt));
		lineEnd = Vector2(400.f, randomY(mt));
		elapsedTime = 0.f;
	}

	currentDegree = Math::FMod(currentDegree + rotateSpeed * InDeltaSeconds, 360.f);	// 주어진 각속도에 따른 현재 점의 위치 각도.
	float sin = 0.f;
	float cos = 0.f;
	Math::GetSinCos(sin, cos, currentDegree);
	point = Vector2(cos, sin) * distance;												// 현재 각도에 따른 단위 벡터를 구해 dist와 곱하여 데카르트 좌표값 계산.
}

// 렌더링 로직을 담당하는 함수
void SoftRenderer::Render2D()
{
	// 렌더링 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& r = GetRenderer();
	const auto& g = Get2DGameEngine();

	// 렌더링 로직의 로컬 변수
	static float radius = 5.f;
	static std::vector<Vector2> circle;

	// 원 구성하는 로직을 사용하여 점 정의.
	if (circle.empty())
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
					circle.push_back(target);
				}
			}
		}
	}

	// 붉은 색으로 점 그리기
	for (auto const& v : circle)
	{
		r.DrawPoint(v + point, LinearColor::Red);		// 원으로 된 점 그리기.
	}

	// 투영할 라인 그리기
	r.DrawLine(lineStart, lineEnd, LinearColor::Black);			// 점에서 투영할 선 그리기.
	r.DrawLine(lineStart, point, LinearColor::Red);				// 점과 투영할 선의 각도를 표현할 선 그리기.

	// 투영된 위치와 거리 계산
	Vector2 unitV = (lineEnd - lineStart).GetNormalize();		// 투영할 선의 단위 벡터.
	Vector2 u = point - lineStart;								// 벡터 v에 투영될 벡터 u.
	Vector2 projV = unitV * (u.Dot(unitV));						// 벡터 v에 투영된 벡터. => 방향은 벡터 v이며 크기는 ||u||cos 이기 때문에, u는 정규화 하지 않고 u와 unitV의 내적값을 구하면 정사영 선의 길이가 된다.
	Vector2 projectedPoint = lineStart + projV;					// 투영된 벡터의 시작점을 지정해줘서 최종적으로 투영된 벡터.
	float distance = (projectedPoint - point).Size();			// 투영된 벡터에서 점까지의 거리를 구하면, 이 값은 해당 점과 선 사이의 거리가 된다.

	// 투영된 점 그리기
	for (auto const& v : circle)
	{
		r.DrawPoint(v + projectedPoint, LinearColor::Magenta);
	}

	// 투영 라인 그리기
	r.DrawLine(projectedPoint, point, LinearColor::Gray);

	// 관련 데이터 화면 출력
	r.PushStatisticText("Point : " + point.ToString());
	r.PushStatisticText("Projected Point : " + projectedPoint.ToString());
	r.PushStatisticText("Distance : " + std::to_string(distance));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
