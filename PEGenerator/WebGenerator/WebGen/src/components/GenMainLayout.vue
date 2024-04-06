<template>
    <el-container style="height: 100vh;">
      <el-header>我的标题</el-header>
  
      <el-container>
        <el-aside width="200px" style="background-color: rgb(238, 241, 246);">
          <el-menu
            default-active="1"
            class="el-menu-vertical-demo"
            @select="handleSelect"
          >
            <el-menu-item index="1">选项1</el-menu-item>
            <el-menu-item index="2">选项2</el-menu-item>
            <el-menu-item index="3">选项3</el-menu-item>
            <!-- 更多选项 -->
          </el-menu>
        </el-aside>
  
        <el-main height="800px">
          <GoJSDemo />
        </el-main>
      </el-container>
    </el-container>
  </template>
  
  <script setup>
  import { ElContainer, ElHeader, ElAside, ElMenu, ElMenuItem, ElMain } from 'element-plus';
  import GoJSDemo from './GoJSDemo.vue'
  
  const handleSelect = (index, indexPath) => {
    console.log("选中的菜单项索引：", index);
    // 根据选中的菜单项索引进行操作，如切换显示的组件内容
  };
  </script>
  
  <style scoped>
  .el-header {
    background-color: #b3c0d1;
    color: #333;
    text-align: center;
    line-height: 60px;
  }
  
  .el-aside {
    color: #333;
  }
  </style>
  