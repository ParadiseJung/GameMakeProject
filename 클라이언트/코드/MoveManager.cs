using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class MoveManager : MonoBehaviour
{
    // Start is called before the first frame update
    [SerializeField]
    private GameObject Chara;
    [SerializeField]
    private GameObject mainCamera;
    [SerializeField]
    private int moveLook; // 0 = left to right , 1 = right to left
    private float moveSpeed;
    private float jumpPower;
    private int jumpCount;
    private bool isGrounded;
    private Rigidbody charaRigidbody;
    private Vector3 vector;

    void Start()
    {
        moveSpeed = 6.5f;
        jumpPower = 12.0f;
        isGrounded = true;
        jumpCount = 0;
        charaRigidbody = GetComponent<Rigidbody>();
    }

    // Update is called once per frame
    void Update()
    {
        CharacterMove();
        rotationChange();
    }

    void CharacterMove() 
    {
        if (Input.GetKey(KeyCode.A)) 
        {
            transform.Translate(Vector3.back * moveSpeed * Time.deltaTime);
        }
        else if (Input.GetKey(KeyCode.D)) 
        {
            transform.Translate(Vector3.forward * moveSpeed * Time.deltaTime);
        }

        if (Input.GetKey(KeyCode.S))
        {

        }
        else if (Input.GetKey(KeyCode.W) && jumpCount < 1 && isGrounded == true)
        {
            jumpCount++;
            charaRigidbody.velocity = new Vector3(0, jumpPower, 0);
        }
    }

    bool rotationCheck() 
    {
        if (charaRigidbody.transform.position.z > mainCamera.transform.position.z)
        {
            moveLook = 0;
            return true;
        }
        else 
        {
            moveLook = 1;
            return false;
        }
    }

    void rotationChange() 
    {
        if (rotationCheck())
        {
            if (moveLook == 0)
            {
                return;
            }
            else
            {
                transform.rotation = Quaternion.Euler(0, 180, 0);
            }
        }
        else 
        {
            if (moveLook == 0)
            {
                transform.rotation = Quaternion.Euler(0, 180, 0);
            }
            else 
            {
                return;
            }
        }
    }

    void OnCollisionEnter(Collision coll) 
    {
        if (coll.collider.tag == "Ground")
        {
            isGrounded = true;
            jumpCount = 0;
        }
    }
}
