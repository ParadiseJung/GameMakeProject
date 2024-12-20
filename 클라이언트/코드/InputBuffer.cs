using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class InputBuffer : MonoBehaviour
{
    public struct InputCommand
    {
        public string inputType; // "Direction", "Attack", "Dash", "JumpStart", "JumpEnd"
        public int direction;    // 방향 값 (1 ~ 9), "Direction"일 때만 사용
        public float timestamp;
    }

    public float bufferTime = 0.5f; // 입력 버퍼 유지 시간 (초)
    private Queue<InputCommand> buffer = new Queue<InputCommand>();

    void OnEnable()
    {
        if (InputManager.Instance == null)
        {
            Debug.LogError("InputManager.Instance is null. Ensure that an InputManager is present in the scene.");
            return;
        }

        InputManager.Instance.OnDirectionInput += AddDirectionInput;
        InputManager.Instance.OnDash += AddDashInput;
        InputManager.Instance.OnAttack += AddAttackInput;
        InputManager.Instance.OnJumpStart += AddJumpStartInput;
        InputManager.Instance.OnJumpEnd += AddJumpEndInput;
    }

    void OnDisable()
    {
        if (InputManager.Instance != null)
        {
            InputManager.Instance.OnDirectionInput -= AddDirectionInput;
            InputManager.Instance.OnDash -= AddDashInput;
            InputManager.Instance.OnAttack -= AddAttackInput;
            InputManager.Instance.OnJumpStart -= AddJumpStartInput;
            InputManager.Instance.OnJumpEnd -= AddJumpEndInput;
        }
    }

    void Update()
    {
        // 오래된 입력 제거
        while (buffer.Count > 0 && Time.time - buffer.Peek().timestamp > bufferTime)
        {
            buffer.Dequeue();
        }
    }

    void AddDirectionInput(int direction)
    {
        InputCommand cmd = new InputCommand
        {
            inputType = "Direction",
            direction = direction,
            timestamp = Time.time
        };
        buffer.Enqueue(cmd);
    }

    void AddDashInput()
    {
        InputCommand cmd = new InputCommand
        {
            inputType = "Dash",
            direction = 0, // 대시는 방향이 아닌 동작에 대한 입력이므로 direction을 0으로 설정
            timestamp = Time.time
        };
        buffer.Enqueue(cmd);
    }

    void AddAttackInput()
    {
        InputCommand cmd = new InputCommand
        {
            inputType = "Attack",
            direction = 0,
            timestamp = Time.time
        };
        buffer.Enqueue(cmd);
    }

    void AddJumpStartInput()
    {
        InputCommand cmd = new InputCommand
        {
            inputType = "JumpStart",
            direction = 0,
            timestamp = Time.time
        };
        buffer.Enqueue(cmd);
    }

    void AddJumpEndInput()
    {
        InputCommand cmd = new InputCommand
        {
            inputType = "JumpEnd",
            direction = 0,
            timestamp = Time.time
        };
        buffer.Enqueue(cmd);
    }

    public List<InputCommand> GetRecentInputs()
    {
        return new List<InputCommand>(buffer);
    }
}
