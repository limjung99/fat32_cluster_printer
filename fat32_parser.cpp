#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#define RESERVED_SECTOR_COUNT_OFFSET 14
#define FAT32_SIZE_32_OFFSET 36
#define BYTE_PER_SECTOR_OFFSET 11
#define SECTOR_PER_CLUSTER_OFFSET 13

void usage(){
    std::cout<<"Usage : ./fat32_parser <image_file>"<<std::endl;
    return;
}

#pragma pack(push, 1) // 컴파일러에게 구조체를 1로 정렬하라고 지시 
struct BootRecord{ //FAT32 부트레코드
    u_int8_t jump_boot_code[3]; //3byte
    u_int8_t oem_name[8]; //8byte
    u_int16_t bytes_per_sector; //2byte
    u_int8_t sector_per_cluster; //1byte
    u_int16_t reserved_sector_count; //2byte
    u_int8_t number_of_fat; //1byte
    u_int16_t root_dir_entry_count; //2byte
    u_int16_t total_sector_16; //2byte
    u_int8_t media; //1byte
    u_int16_t fat_size_16; //2byte
    u_int16_t sector_per_track; //2byte
    u_int16_t number_of_heads; //2byte
    u_int32_t hidden_sector; //4byte
    u_int32_t total_sector_32; //4byte
    u_int32_t fat_size_32; //4byte
    u_int16_t ext_flags; // 2bytes
    u_int16_t file_system_version; // 2bytes
    u_int32_t root_dir_cluster; //4bytes
    u_int16_t file_system_info; //2bytes
    u_int16_t boot_record_backup_sec; //2bytes
    u_int8_t reserved[12]; //12bytes
    u_int8_t drive_number; //1bytes
    u_int8_t reserved1; //1bytes
    u_int8_t boot_signature; //1bytes
    u_int32_t volume_id; //4bytes
    u_int8_t volume_label[11]; // 11bytes
    u_int64_t file_system_type; // 8bytes
    // total : 90 bytes
};
#pragma pack(pop)

u_int cluster_size;

int main(int argc,char* argv[]){

    if(argc<2){
        usage();
        return -1;
    }

    std::ifstream fs(argv[1]);
    BootRecord bootrecord;
    
    if(!fs.is_open()){
        std::cout<<"Can't Open imagefile"<<std::endl;
        return -1;
    }
    fs.read((char*)&bootrecord,sizeof(BootRecord));
    //byte per sector 계산
    u_int16_t byte_per_sector = bootrecord.bytes_per_sector;
    //sector per cluster 계산 및 클러스터크기 계산
    u_int8_t sector_per_cluster = bootrecord.sector_per_cluster;
    cluster_size = byte_per_sector * sector_per_cluster;
    // fat 오프셋 계산 
    u_int16_t fat_offset = bootrecord.reserved_sector_count; //예약영역크기 == fat필드의 시작 오프셋 섹터  -> 왜 엔디안이 정렬되있지??
    u_int start_of_fat = fat_offset * 512;
    // fat 사이즈 계산
    u_int32_t fat_size_32 = bootrecord.fat_size_32;
    fat_size_32 = fat_size_32*512;
    u_int32_t fat_fields[fat_size_32/4];
    //fatoffset으로 이동 후 FAT사이즈만큼 Read
    fs.seekg(start_of_fat);
    fs.read((char*)&fat_fields,fat_size_32);
    printf("%ld\n",sizeof(fat_fields)); // 왜 sizeof로 보면 311808이 잡히는데 lldb에서는 5로 잡히지?
    //입력받은 클러스터만큼 링크드 리스트구조를 이동하면서 print
    //4바이트마다 한개의 클러스터임 
    u_int fat_num = atoi(argv[2]);
    while(true){
        printf("%d ",fat_num);
        u_int next = fat_fields[fat_num];
        if(next==0x0fffffff) break;
        fat_num = next;
    } 
    fs.close();
}