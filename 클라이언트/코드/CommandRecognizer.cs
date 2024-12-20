using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CommandRecognizer : MonoBehaviour
{
    public InputBuffer inputBuffer;
    public CharacterController3D characterController;

    private List<string> forwardAttackCommand = new List<string> { "Direction:6", "Attack" };
    private List<string> jumpAttackCommand = new List<string> { "JumpStart", "Attack" };

    private float jumpStartTime = 0f;
    private bool isJumping = false;

    void OnEnable()
    {
        if (InputManager.Instance == null)
        {
            Debug.LogError("InputManager.Instance is null. Ensure that an InputManager is present in the scene.");
            return;
        }

        InputManager.Instance.OnAttack += CheckForCommands;
        InputManager.Instance.OnDirectionInput += HandleDirectionInput;
        InputManager.Instance.OnJumpStart += OnJumpStart;
        InputManager.Instance.OnJumpEnd += OnJumpEnd;

        Debug.Log("CommandRecognizer OnEnable executed successfully.");
    }

    void OnDisable()
    {
        if (InputManager.Instance != null)
        {
            InputManager.Instance.OnAttack -= CheckForCommands;
            InputManager.Instance.OnDirectionInput -= HandleDirectionInput;
            InputManager.Instance.OnJumpStart -= OnJumpStart;
            InputManager.Instance.OnJumpEnd -= OnJumpEnd;
        }
    }

    void CheckForCommands()
    {
        List<InputBuffer.InputCommand> recentInputs = inputBuffer.GetRecentInputs();
        List<string> inputSequence = new List<string>();

        foreach (var cmd in recentInputs)
        {
            if (cmd.inputType == "Direction")
                inputSequence.Add($"Direction:{cmd.direction}");
            else if (cmd.inputType == "Attack")
                inputSequence.Add("Attack");
        }

        if (IsSequenceMatch(inputSequence, forwardAttackCommand))
        {
            ExecuteForwardAttack();
            return;
        }

    }

    bool IsSequenceMatch(List<string> inputSequence, List<string> commandSequence)
    {
        if (inputSequence.Count < commandSequence.Count)
            return false;

        for (int i = 0; i < commandSequence.Count; i++)
        {
            if (inputSequence[inputSequence.Count - commandSequence.Count + i] != commandSequence[i])
                return false;
        }

        return true;
    }

    void HandleDirectionInput(int direction)
    {

    }

    void ExecuteForwardAttack()
    {
        Debug.Log("Forward Attack Executed!");
        if (characterController != null)
        {
            characterController.SpecialForwardAttack();
        }
        else
            Debug.LogError("CharacterController3D is not assigned in CommandRecognizer.");
    }

    void OnJumpStart()
    {
        if (!isJumping)
        {
            isJumping = true;
            jumpStartTime = Time.time;
            Debug.Log("Jump Started.");
        }
    }

    void OnJumpEnd()
    {
        if (isJumping)
        {
            float jumpDuration = Time.time - jumpStartTime;
            if (jumpDuration < 0.15f)
                jumpDuration = 0.15f;
            ExecuteJumpWithDuration(jumpDuration);
            isJumping = false;
            Debug.Log("Jump Ended. Duration: " + jumpDuration);
        }
    }

    void ExecuteJumpWithDuration(float duration)
    {
        float maxJumpDuration = 0.2f;
        float normalizedDuration = Mathf.Clamp01(duration / maxJumpDuration);
        characterController.Jump(normalizedDuration);
    }

}
