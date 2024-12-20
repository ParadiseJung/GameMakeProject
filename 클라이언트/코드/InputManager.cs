using System.Collections;
using System.Collections.Generic;
using System;
using UnityEngine;

public class InputManager : MonoBehaviour
{
    public static InputManager Instance;

    public event Action<int> OnDirectionInput;
    public event Action OnDash;
    public event Action OnAttack; // 공격 이벤트
    public event Action OnJumpStart;
    public event Action OnJumpEnd;

    private float lastATime = -1f;
    private float lastDTime = -1f;
    private float doubleTapTime = 0.3f;

    void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            DontDestroyOnLoad(gameObject);
            Debug.Log("InputManager Instance set.");
        }
        else
        {
            Destroy(gameObject);
        }
    }

    void Update()
    {
        if (NetworkManager.Instance == null || !NetworkManager.Instance.gameStart || NetworkManager.Instance.gameEnd)
        {
            // 게임 시작 전이나 게임 종료 후에는 입력 처리 X
            return;
        }

        // 방향 입력
        int direction = GetDirection();
        OnDirectionInput?.Invoke(direction);

        // 대시 감지
        DetectDash();

        // 점프
        if (Input.GetButtonDown("Jump"))
        {
            OnJumpStart?.Invoke();
        }
        if (Input.GetButtonUp("Jump"))
        {
            OnJumpEnd?.Invoke();
        }

        // 공격키 (J키)
        if (Input.GetKeyDown(KeyCode.J))
        {
            OnAttack?.Invoke();
        }
    }

    public int GetDirection()
    {
        bool w = Input.GetKey(KeyCode.W);
        bool a = Input.GetKey(KeyCode.A);
        bool s = Input.GetKey(KeyCode.S);
        bool d = Input.GetKey(KeyCode.D);

        if (w && a) return 7;
        if (w && d) return 9;
        if (s && a) return 1;
        if (s && d) return 3;
        if (w) return 8;
        if (a) return 4;
        if (s) return 2;
        if (d) return 6;
        return 5;
    }

    void DetectDash()
    {
        if (Input.GetKeyDown(KeyCode.A))
        {
            if (Time.time - lastATime < doubleTapTime)
            {
                OnDash?.Invoke();
                // 대시는 서버로도 전송될 필요가 있을 수 있음
                // 예: NetworkManager.Instance.SendDash(-1);
            }
            lastATime = Time.time;
        }

        if (Input.GetKeyDown(KeyCode.D))
        {
            if (Time.time - lastDTime < doubleTapTime)
            {
                OnDash?.Invoke();
                // 대시는 서버로도 전송될 필요가 있을 수 있음
                // 예: NetworkManager.Instance.SendDash(1);
            }
            lastDTime = Time.time;
        }
    }
}