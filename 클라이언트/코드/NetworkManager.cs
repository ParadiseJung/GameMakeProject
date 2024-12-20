using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.Net.Sockets;
using System.Threading;
using System;
using UnityEngine.SceneManagement;
using System.Collections.Concurrent;

public enum PacketHeader : byte
{
    NON_HEAD,
    RES_START,
    REQ_COM,
    RES_COM,
    REQ_RO,
    RES_RO,
    RES_END,
    REQ_QUIT
};

public enum RoundOver : byte
{
    None = 0,
    Win = 1,
    Lose = 2,
    Draw = 3
};

public class NetworkManager : MonoBehaviour
{
    public static NetworkManager Instance;
    public InputManager inputManager;

    private Socket sock;
    private Thread recvThread;
    public bool running = false;

    public int myRoomId = 0;
    public bool gameStart = false;
    public bool gameEnd = false;

    float elapsedTime = 0f;

    private char currentDirection = (char)5;
    private char currentAttack = (char)0;

    // RES_COM 패킷 입력 상태 저장
    private char lastP1Move = (char)5, lastP1Attack = (char)0;
    private char lastP2Move = (char)5, lastP2Attack = (char)0;

    // 이벤트 큐: 패킷 수신 스레드에서 메인 스레드로 액션을 전달
    private ConcurrentQueue<Action> eventQueue = new ConcurrentQueue<Action>();

    private bool needSceneLoad = false;
    private bool gameSceneLoaded = false;
    // Player 컨트롤러 참조
    private CharacterController3D p1Controller;
    private CharacterController3D p2Controller;

    // 연결 상태
    private bool isConnecting = false;

    void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(this.gameObject);
            SceneManager.sceneLoaded += OnSceneLoaded;
        }
        else
        {
            Destroy(gameObject);
        }
    }

    // 서버 연결 시도 메서드
    // 성공: true 반환
    // 실패: false 반환
    public bool ConnectToServer(string ip, int port)
    {
        if (isConnecting)
        {
            Debug.Log("이미 서버 연결을 시도 중입니다.");
            return false;
        }

        try
        {
            sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            sock.Connect(ip, port);
        }
        catch (Exception e)
        {
            Debug.LogError("서버 연결 실패: " + e.Message);
            return false;
        }

        running = true;
        recvThread = new Thread(RecvThreadProc);
        recvThread.Start();
        isConnecting = true; // 연결 시도 중으로 설정
        return true;
    }

    void OnDestroy()
    {
        running = false;
        if (recvThread != null && recvThread.IsAlive) recvThread.Join();
        if (sock != null && sock.Connected) sock.Close();
        SceneManager.sceneLoaded -= OnSceneLoaded;
    }

    void Start()
    {
        if (InputManager.Instance != null)
        {
            InputManager.Instance.OnDirectionInput += OnDirectionInput;
            InputManager.Instance.OnAttack += OnAttackInput;
            InputManager.Instance.OnJumpStart += OnJumpStartInput;
            InputManager.Instance.OnJumpEnd += OnJumpEndInput;
        }
    }

    void OnSceneLoaded(Scene scene, LoadSceneMode mode)
    {
        if (scene.name == "Game")
        {
            gameSceneLoaded = true;
            // Game씬 로드 후 카운트다운 초기화
            // Timer는 Game씬의 스크립트에서 초기화할 수 있음
            // Player1, Player2 찾기
            p1Controller = GameObject.Find("Player1")?.GetComponent<CharacterController3D>();
            p2Controller = GameObject.Find("Player2")?.GetComponent<CharacterController3D>();
            inputManager = GameObject.Find("InputManager")?.GetComponent<InputManager>();
        }
    }

    void OnDirectionInput(int direction)
    {
        currentDirection = (char)direction;
        SendReqComPacket();
    }

    void OnAttackInput()
    {
        currentAttack = (char)1;
        SendReqComPacket();
    }

    void OnJumpStartInput()
    {
        // JumpStart 관련 패킷을 보내려면 별도 로직 추가
        // 예: SendJumpStartPacket();
    }

    void OnJumpEndInput()
    {
        // JumpEnd 관련 패킷을 보내려면 별도 로직 추가
        // 예: SendJumpEndPacket();
    }

    void Update()
    {
        // 이벤트 큐 처리
        while (eventQueue.TryDequeue(out Action action))
        {
            action?.Invoke();
        }

        if (!running) return;

        // 씬 전환 플래그 체크
        if (needSceneLoad)
        {
            SceneManager.LoadScene("Game");
            needSceneLoad = false;
        }

        if (!gameStart || gameEnd || !gameSceneLoaded) return;

        // 초/프레임 카운트다운은 Game씬에서 별도로 처리

        // 서버에 입력 전송
        // SendReqComPacket()을 InputManager에서 호출하도록 수정
    }

    void RecvThreadProc()
    {
        byte[] buf = new byte[1024];
        while (running)
        {
            int recvBytes = 0;
            try
            {
                recvBytes = sock.Receive(buf);
            }
            catch
            {
                running = false;
                break;
            }
            if (recvBytes <= 0)
            {
                running = false;
                break;
            }
            HandleReceivedPacket(buf, recvBytes);
        }
    }

    void HandleReceivedPacket(byte[] buffer, int bytesTransferred)
    {
        if (bytesTransferred < 2 + 1 + 2) return;
        short length = BitConverter.ToInt16(buffer, 0);
        if (length != bytesTransferred) return;
        PacketHeader header = (PacketHeader)buffer[2];

        switch (header)
        {
            case PacketHeader.RES_START:
                myRoomId = buffer[3];
                gameStart = true;
                Debug.Log("[RecvThread] RES_START: myRoomId=" + myRoomId + " gameStart=true");
                isConnecting = false; // 연결 시도 완료
                // 씬 로드를 이벤트 큐에 추가
                eventQueue.Enqueue(() =>
                {
                    needSceneLoad = true;
                });
                break;

            case PacketHeader.RES_COM:
                {
                    char p1move = (char)buffer[4];
                    char p1attack = (char)buffer[5];
                    char p2move = (char)buffer[6];
                    char p2attack = (char)buffer[7];
                    char sec = (char)buffer[8];
                    char frame = (char)buffer[9];

                    // 받은 입력을 변수에 저장
                    lastP1Move = p1move;
                    lastP1Attack = p1attack;
                    lastP2Move = p2move;
                    lastP2Attack = p2attack;

                    // 메인 스레드에서 캐릭터 업데이트를 수행하기 위해 eventQueue에 액션 추가
                    eventQueue.Enqueue(() =>
                    {
                        if (p1Controller != null)
                            p1Controller.UpdateFromServer(lastP1Move, lastP1Attack);
                        if (p2Controller != null)
                            p2Controller.UpdateFromServer(lastP2Move, lastP2Attack);
                    });
                }
                break;

            case PacketHeader.RES_RO:
                {
                    char p1Num = (char)buffer[3];
                    RoundOver p1ROState = (RoundOver)buffer[4];
                    char p2Num = (char)buffer[5];
                    RoundOver p2ROState = (RoundOver)buffer[6];
                    char checkState = (char)buffer[7];

                    Debug.Log("[RecvThread] RES_RO: p1RO=" + p1ROState + " p2RO=" + p2ROState);
                    gameStart = false;
                }
                break;

            case PacketHeader.RES_END:
                Debug.Log("[RecvThread] Game is End");
                gameEnd = true;
                break;
        }
    }

    void SendReqComPacket()
    {
        if (sock == null || !sock.Connected) return;

        // 패킷 구조:
        // length(2바이트), header(1바이트), roomId(1바이트), move(1바이트), attack(1바이트), sec(1바이트), frame(1바이트), delimiter(2바이트)
        short length = 2 + 1 + 1 + 1 + 1 + 1 + 1 + 2; // 총 10바이트
        byte[] sendBuf = new byte[length];

        // length
        BitConverter.GetBytes(length).CopyTo(sendBuf, 0);
        // header
        sendBuf[2] = (byte)PacketHeader.REQ_COM;
        // roomId
        sendBuf[3] = (byte)myRoomId;
        // move
        sendBuf[4] = (byte)inputManager.GetDirection();
        // attack
        sendBuf[5] = (byte)currentAttack;
        // sec and frame could be set to 0 or based on game state
        sendBuf[6] = 0; // 예시
        sendBuf[7] = 0; // 예시
        // delimiter
        short delimiter = 0xFF;
        BitConverter.GetBytes(delimiter).CopyTo(sendBuf, 8);

        // 패킷 송신
        try
        {
            sock.Send(sendBuf);
            Debug.Log($"[TestClient] REQ_COM Packet sent. move={currentDirection}, attack={currentAttack}");
        }
        catch (Exception e)
        {
            Debug.LogError("패킷 전송 실패: " + e.Message);
        }

        // 공격 후 초기화
        if (currentAttack == (char)1)
            currentAttack = (char)0;
    }
}
