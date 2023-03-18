# catCareBot
2023 종합설계 고양이 케어 로봇 프로그램 및 관련 자료 저장소. Nucleo-F411RE 보드 및 라즈베리파이를 씀

진행상황

라즈베리파이: 거의 100% 완료. (멀티스레드 소켓통신, 고양이 검출, mjpg 스트리머 서버 구동, mjpg 서버에서 가져와서 검출)

STM32: 통신 완료, 다만 오류(밀림 발생)시 UART 초기화하는 코드 있으면 좋을 듯.. 
L298N 모터 드라이버 구동 프로그램은 제대로 작동함.

core 프로그램이 쓸데없이 커지고 opman과 scheduler는 굳이 core에서 분리할 필요가 없어보임

core에서 놀이 패턴 구현, 수동 운전 등등의 부분은 app.c&h로 빼버리고, opman과 scheduler는 core에 통합하는 게 나아보임.
