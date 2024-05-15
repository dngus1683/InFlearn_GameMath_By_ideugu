
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
float currentShear = 0.f;		// shearing(밀기 => 평행사변형 꼴 만들기)
float currentScale = 10.f;
float currentDegree = 0.f;

// 게임 로직을 담당하는 함수
void SoftRenderer::Update2D(float InDeltaSeconds)
{
	// 게임 로직에서 사용하는 모듈 내 주요 레퍼런스
	auto& g = Get2DGameEngine();
	const InputManager& input = g.GetInputManager();

	// 게임 로직의 로컬 변수
	static float shearSpeed = 2.f;
	static float scaleMin = 5.f;
	static float scaleMax = 20.f;
	static float scaleSpeed = 20.f;
	static float rotateSpeed = 180.f;

	float deltaShear = input.GetAxis(InputAxis::XAxis) * shearSpeed * InDeltaSeconds;
	float deltaScale = input.GetAxis(InputAxis::ZAxis) * scaleSpeed * InDeltaSeconds;
	float deltaDegree = input.GetAxis(InputAxis::WAxis) * rotateSpeed * InDeltaSeconds;

	// 물체의 최종 상태 설정
	currentShear += deltaShear;
	currentScale = Math::Clamp(currentScale + deltaScale, scaleMin, scaleMax);
	currentDegree += deltaDegree;
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

	static Vector2 pivot(200.f, 0.f);		// 두 하트 간의 간격. 

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


	// ***************************** 하트 함수에 scale, rotate, shear 변환을 행렬로 한꺼번에 적용한 결과와, 역행렬을 변환된 값에 적용한 결과 비교하기 **************************
	// Scale Matrix
	Vector2 sBasis1(currentScale, 0.f);
	Vector2 sBasis2(0.f, currentScale);
	Matrix2x2 sMatrix(sBasis1, sBasis2);

	// Rotate Matrix
	float sin = 0.f;
	float cos = 0.f;
	Math::GetSinCos(sin, cos, currentDegree);
	Vector2 rBasis1(cos, sin);
	Vector2 rBasis2(-sin, cos);
	Matrix2x2 rMatrix(rBasis1, rBasis2);

	// Shear Matrix		
	// sh = [ 1, a ]
	//		[ 0, 1 ]
	Vector2 shBasis1(1.f, 0.f);
	Vector2 shBasis2(currentShear, 1.f);
	Matrix2x2 shMatrix(shBasis1, shBasis2);

	// 합성행렬
	// shear 변환 역시 linear Transformation에 해당되기 때문에, 행렬 곱 순서는 중요치 않다.
	Matrix2x2 tMatrix = sMatrix * rMatrix * shMatrix;

	// Scale Matrix의 역행렬
	// 각 scale 역수
	Vector2 isBasis1(1.f/currentScale, 0.f);
	Vector2 isBasis2(0.f, 1.f/currentScale);
	Matrix2x2 isMatrix(isBasis1, isBasis2);

	// Rotate Matrix의 역행렬
	// rotation의 전치
	Matrix2x2 irMatrix = rMatrix.Transpose();

	// Shear Matrix의 역행렬
	// 다른 축 값을 더한 만큼(잡아 늘린만큼) 빼기
	// sh^-1 = [ 1, -a ]
	//		   [ 0,  1 ]
	Vector2 ishBasis1(1.f, 0.f);
	Vector2 ishBasis2(-currentShear, 1.f);
	Matrix2x2 ishMatrix(ishBasis1, ishBasis2);

	// 역행렬의 합성행렬
	Matrix2x2 itMatrix = ishMatrix * irMatrix * isMatrix ;
	

	rad = 0.f;
	for (auto const& v : hearts)
	{
		// 왼쪽 하트(Transformation 적용)
		Vector2 left = tMatrix * v;
		r.DrawPoint(left - pivot, hsv.ToLinearColor());

		// 오른쪽 하트(왼쪽 하트에 추가로 역행렬 적용)
		Vector2 right = itMatrix * left;
		r.DrawPoint(right + pivot, hsv.ToLinearColor());

		hsv.H = rad / Math::TwoPI;
		rad += increment;
	}

	// 현재 밀기, 스케일, 회전각을 화면에 출력
	r.PushStatisticText(std::string("Shear : ") + std::to_string(currentShear));
	r.PushStatisticText(std::string("Scale : ") + std::to_string(currentScale));
	r.PushStatisticText(std::string("Degree : ") + std::to_string(currentDegree));
}

// 메시를 그리는 함수
void SoftRenderer::DrawMesh2D(const class DD::Mesh& InMesh, const Matrix3x3& InMatrix, const LinearColor& InColor)
{
}

// 삼각형을 그리는 함수
void SoftRenderer::DrawTriangle2D(std::vector<DD::Vertex2D>& InVertices, const LinearColor& InColor, FillMode InFillMode)
{
}
