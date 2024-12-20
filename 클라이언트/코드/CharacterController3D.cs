using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CharacterController3D : MonoBehaviour
{
    public float moveSpeed = 5f;
    public float jumpForce = 5f; // 기본 점프 힘
    public LayerMask groundMask; // 땅을 인식하기 위한 레이어 마스크
    public Transform groundCheck; // 땅 체크를 위한 Transform
    public float groundCheckRadius = 0.2f; // 땅 체크 범위

    public float minZ = -5f; // 좌측 이동 제한
    public float maxZ = 5f;  // 우측 이동 제한

    private Rigidbody rb;
    private Animator animator;
    private bool isGrounded;
    private bool isDashing = false; // 대시 상태 확인

    void Start()
    {
        rb = GetComponent<Rigidbody>();
        rb.constraints = RigidbodyConstraints.FreezeRotation; // 회전 제한, 이동은 자유롭게
        animator = GetComponent<Animator>();
    }

    void Update()
    {
        animator.SetBool("IsGrounded", isGrounded);
    }

    void FixedUpdate()
    {
        isGrounded = Physics.CheckSphere(groundCheck.position, groundCheckRadius, groundMask);
    }

    // 서버로부터 받은 move, attack 정보를 해석하여 캐릭터 이동/행동 적용
    public void UpdateFromServer(char move, char attack)
    {
        // move: 1~9
        // attack: 0 또는 1

        Vector3 pos = transform.position;
        // move에 따라 방향 해석
        bool w = (move == '7' || move == '8' || move == '9');
        bool a = (move == '1' || move == '4' || move == '7');
        bool s = (move == '1' || move == '2' || move == '3');
        bool d = (move == '3' || move == '6' || move == '9');

        float x = 0f;
        // A=left(-1), D=right(+1)
        if (a && !d) x = -1f;
        if (d && !a) x = 1f;
        // W키 포함하면 점프 시도
        if (w && isGrounded)
        {
            Jump(1f);
        }

        // 수평 이동
        pos.z += x * moveSpeed * Time.deltaTime;
        pos.z = Mathf.Clamp(pos.z, minZ, maxZ);
        rb.MovePosition(pos);

        float speed = Mathf.Abs(x);
        animator.SetFloat("Speed", speed);

        // attack 실행
        if (attack == (char)1)
        {
            Attack();
        }
    }

    // 점프 메서드: normalizedDuration은 0~1 범위
    public void Jump(float normalizedDuration)
    {
        if (isGrounded)
        {
            float appliedJumpForce = jumpForce * normalizedDuration;
            rb.velocity = new Vector3(rb.velocity.x, 0, rb.velocity.z); // Y축 속도 초기화
            rb.AddForce(Vector3.up * appliedJumpForce, ForceMode.Impulse);
            animator.SetTrigger("Jump");
        }
    }

    // 대시 메서드: 좌우로 빠르게 이동
    public void Dash(float direction)
    {
        if (isGrounded)
        {
            StartCoroutine(DashRoutine(direction));
        }
    }

    private IEnumerator DashRoutine(float direction)
    {
        isDashing = true;
        float dashDuration = 0.2f; // 대시 지속 시간
        float dashSpeed = moveSpeed * 2.5f; // 대시 속도
        float dashTime = 0f;

        while (dashTime < dashDuration)
        {
            dashTime += Time.fixedDeltaTime;
            Vector3 dashMove = new Vector3(0, 0, direction * dashSpeed * Time.fixedDeltaTime);
            rb.MovePosition(rb.position + dashMove);
            yield return null;
        }

        isDashing = false;
    }

    // 기본 공격 메서드
    public void Attack()
    {
        Debug.Log("Basic Attack!");
        animator.SetTrigger("Attack");
    }

    // 특수 전진 공격 메서드
    public void SpecialForwardAttack()
    {
        Debug.Log("Special Forward Attack!");
        animator.SetTrigger("SpecialAttack");
    }

    // 점프 후 공격 메서드
    public void JumpAndAttack()
    {
        if (isGrounded)
        {
            Jump(1f); // 최대 점프 높이
            Attack();
        }
    }
}
