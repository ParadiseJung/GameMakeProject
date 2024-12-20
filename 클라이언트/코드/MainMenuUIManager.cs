using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using UnityEngine.SceneManagement;

public class MainMenuUIManager : MonoBehaviour
{
    public Button startButton;
    public Button quitButton;

    private bool isConnecting = false; // 중복 클릭 방지용

    void Start()
    {
        startButton.onClick.AddListener(OnStartGameClicked);
        quitButton.onClick.AddListener(OnQuitGameClicked);
    }

    void OnStartGameClicked()
    {
        if (isConnecting)
        {
            Debug.Log("이미 서버에 연결 시도 중입니다...");
            return;
        }

        isConnecting = true;

        bool success = NetworkManager.Instance.ConnectToServer("127.0.0.1", 9001);
        if (!success)
        {
            Debug.Log("서버 연결 실패!");
            isConnecting = false; // 실패하면 다시 시도할 수 있게
        }
        else
        {
            Debug.Log("서버 연결 성공, RES_START 패킷 대기중...");
        }
    }

    void OnQuitGameClicked()
    {
        Application.Quit();
    }
}
