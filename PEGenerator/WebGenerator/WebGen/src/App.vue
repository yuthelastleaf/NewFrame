<script setup>
import GenMainLayout from './components/GenMainLayout.vue'
</script>

<template>
  <!-- <div>
    <a href="https://vitejs.dev" target="_blank">
      <img src="/vite.svg" class="logo" alt="Vite logo" />
    </a>
    <a href="https://vuejs.org/" target="_blank">
      <img src="./assets/vue.svg" class="logo vue" alt="Vue logo" />
    </a>
  </div> -->
  <!-- <HelloWorld msg="Vite + Vue" /> -->
  <div class="full-screen">
    <GenMainLayout />
  </div>
</template>

<style scoped>
/* .logo {
  height: 6em;
  padding: 1.5em;
  will-change: filter;
  transition: filter 300ms;
}
.logo:hover {
  filter: drop-shadow(0 0 2em #646cffaa);
}
.logo.vue:hover {
  filter: drop-shadow(0 0 2em #42b883aa);
} */
  body, html {
    margin: 0;
    padding: 0;
    height: 100%;
  }
  body {
    display: grid; /* 应用 Grid 布局 */
    min-height: 100vh; /* 最小高度为视口高度 */
  }
/* 设置 div 铺满整个屏幕 */
.full-screen {
    width: 100%; /* 视口宽度 */
    height: 100%; /* 视口高度 */
  }
</style>
