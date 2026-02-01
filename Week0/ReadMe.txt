## 추가 기능 구현

### 각속도 운동(Angular Velocity)

- UBall::MoveAngular() 함수에서 구현
- Angular Velocity 체크박스 활성화 시 기존 멤버 변수인 Velocity값을 각속도로 변환해 (0, 0, 0)을 기준으로 등각속도 운동 수행

- 평면상에서 이루어지는 물체의 운동이기 때문에 omega = r x v / (|r|^2) 공식을 사용해 각속도 계산
  - r: 원점으로부터 물체의 거리, v: 물체의 속도

- 위에서 구한 각속도 값을 기반으로 물체의 속도 계산(v = w x r)