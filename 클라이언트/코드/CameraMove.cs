using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CameraMove : MonoBehaviour
{
    public GameObject chara1;
    public GameObject chara2;

    public float baseDistance = 3f; // 기본 카메라 거리
    public float maxZoomOutDistance = 7f; // 최대 줌 아웃 거리
    public float zoomSpeed = 4f; // 줌 아웃 속도

    private void Start()
    {

    }

    // Update is called once per frame
    private void Update()
    {
        AdjustCameraPosition();
    }

    private void AdjustCameraPosition()
    {
        // 두 캐릭터 간의 거리 계산
        float distance = Vector3.Distance(chara1.transform.position, chara2.transform.position);

        // 카메라의 새로운 위치를 계산
        Vector3 newCameraPosition = this.transform.position;

        // 기본 거리와 캐릭터 간 거리 비교
        if (distance > baseDistance)
        {
            // 카메라의 X 위치를 거리의 비율에 따라 조정
            float zoomOutFactor = Mathf.Clamp(distance / baseDistance, 1, maxZoomOutDistance / baseDistance);
            newCameraPosition.x = baseDistance * zoomOutFactor;
        }
        else
        {
            // 기본 거리로 되돌리기
            newCameraPosition.x = baseDistance;
        }

        // Z 위치는 두 캐릭터의 평균 Z 좌표로 설정
        newCameraPosition.z = (chara1.transform.position.z + chara2.transform.position.z) / 2;

        // 카메라 위치를 부드럽게 이동
        this.transform.position = Vector3.Lerp(this.transform.position, newCameraPosition, Time.deltaTime * zoomSpeed);
    }
}
